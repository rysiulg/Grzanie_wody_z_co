// Sterowanie grzeniem wody z co
// Arduino MiniPro->ESP8266 v.2.7.1 arduino 1.8.12 i zmodyfikowany Filesystem do 3MBSketchOTA/SPIFF1MB dla NodeMCU1.0 (ESP 12E) !!!!!!!!!!!!!!!!!!!!!!!!
// v.1.0 20190922start
// v.1.1 -modyfikacja przekaznika 20190923 -przepiecie by dzialal domyślnie aktywnie bez zasilania oraz odłączanie pompy do wody poprzez aktywacje przekaznika
// v.2 -modyfikacja do ESP8266 Wifi+Supla+http update+SSR zamiast przekaznika
// v2.02 modyfikacje dla supla 2.3 i enchanced config +czujnik co i bme085 (cisnienie i temp)
// v2.03 modyfikacje zmiana interfejsu www
// v2.05 obsluga licznika energii z zapisaniem np. co 15 minut w SPIFF i obsluga definicji SN 18B20 z config -nie startowalo jeszcze gdy zlapalo wifi...
// v2.10 powrot do ESP 2.6.1 httpupdater robil crash po aktualizacji w wersji 2.7
// 2.11 spiecie update menager i async web
// v2.12 skorygowano wylaczanie/wlaczanie pomp i dodany link uptime
// v2.13 kalibracja gas sensor
// v2.14 dopisane force by bytton (zuzycie energii)
// v2.15 wdresety, ustawienie wymuszania pomp w zaleznosci od przycisku i dodatkowo gdy temp na zewnatrz srednia z czujnikow < forceCObelow
// v2.15a-i drobne poprawki logiki
// v2.16 -buczalo gdy temp co byla <15 -zmiana na <1, zmniejszona wartosc forceCObelow do 7st
// v3.00 -move SUPLA to MQTT, upgrade Json 5->6, enable stack protection in esp config, arduino po aktualizacji traktuje string="" jako 0.0000 i przez to autoryzacja sie wysypuje w webhandler
// v3.01 -drobne poprawki, wywalone z bliblioteki ESPAsyncWebServer linijek by usunac kom. o autoryzacjach  w WebHandlerImpl.h //    if((_username != "" && _password != "") && !request->authenticate(_username.c_str(), _password.c_str()))  return request->requestAuthentication();//44444444444444444444444444444444444444444446666666666666666666666666666666666666666666666666666666666
// v3.02 -uruchomione z config na guziku
// v3.02a -aktualizacja ikonek mqtt, dodany prefix mqtt oraz uzupelnione opcje device w mqtt
// v3.02b -dolozony parametr opozniajacy o 1 godz. wylaczenie wymuszenia zmiany priotytetu pompy z woda na Co gdy temp < od temp aktywacji pomp
//  Copyright (C) MARM.pl
// v3.02e -poprawa display i dodany dwukropek do temps
// v202205 remove arduinojson, HomeassistantArduino -implement own

//*************************************************
// pins

//#include "SPIFFS.h"
//#include <LittleFS.h>
//#define SPIFFS LITTLEFS

//*************************************************0x28, 0xFF, 0x4B, 0x72, 0x80, 0x14, 0x02, 0x77
#include "main.h"

//*************************************************

#include "funkcje.h"
#include "configPortal.h"
#include "common_functions.h"

//*************************************************

/*
 *  check_wifi will process wifi reconnecting managing
 */
void check_wifi()
{
  if ((WiFi.status() != WL_CONNECTED) || (!WiFi.localIP()))
  {
    /*
     *  if we are not connected to an AP
     *  we must be in softAP so respond to DNS
     */
    dnsServer.processNextRequest();

    /* we need to stop reconnecting to a configured wifi network if there is a hotspot user connected
     *  also, do not disconnect if wifi network scan is active
     */
    if ((ssid[0] != '\0') && (WiFi.status() != WL_DISCONNECTED) && (WiFi.scanComplete() != -1) && (WiFi.softAPgetStationNum() > 0))
    {
      log_message((char *)"WiFi lost, but softAP station connecting, so stop trying to connect to configured ssid...");
      WiFi.disconnect(true);
    }

    /*  only start this routine if timeout on
     *  reconnecting to AP and SSID is set
     */
    if ((ssid[0] != '\0') && ((unsigned long)(millis() - lastWifiRetryTimer) > WIFIRETRYTIMER))
    {
      lastWifiRetryTimer = millis();
      if (WiFi.softAPSSID() == "")
      {
        log_message((char *)"WiFi lost, starting setup hotspot...");
        WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
        WiFi.softAP(me_lokalizacja.c_str());
      }
      if ((WiFi.status() == WL_DISCONNECTED) && (WiFi.softAPgetStationNum() == 0))
      {
        log_message((char *)"Retrying configured WiFi, ...");
        if (pass[0] == '\0')
        {
          WiFi.begin(ssid);
        }
        else
        {
          WiFi.begin(ssid, pass);
        }
      }
      else
      {
        log_message((char *)"Reconnecting to WiFi failed. Waiting a few seconds before trying again.");
        WiFi.disconnect(true);
      }
    }
  }
  else
  { // WiFi connected
    if (WiFi.softAPSSID() != "")
    {
      //      log_message((char *)"WiFi (re)connected, shutting down hotspot...");
      //      WiFi.softAPdisconnect(true);
      //      MDNS.notifyAPChange();
#ifndef ESP32
      //     experimental::ESP8266WiFiGratuitous::stationKeepAliveSetIntervalMs(5000); // necessary for some users with bad wifi routers
#endif
    }

    if (firstConnectSinceBoot)
    { // this should start only when softap is down or else it will not work properly so run after the routine to disable softap
      firstConnectSinceBoot = false;
      lastmqtt_reconnect = 0; // initiate mqtt connection asap


      if (ssid[0] == '\0')
      {
        log_message((char *)"WiFi connected without SSID and password in settings. Must come from persistent memory. Storing in settings.");
        WiFi.SSID().toCharArray(ssid, 40);
        WiFi.psk().toCharArray(pass, 40);
        SaveConfig(); // save to config file
      }
    }

    /*
       always update if wifi is working so next time on ssid failure
       it only starts the routine above after this timeout
    */
    lastWifiRetryTimer = millis();

    // Allow MDNS processing
    //    MDNS.update();
  }
}

void setupMqtt()
{
  mqttclient.setBufferSize(2048);
  mqttclient.setSocketTimeout(10);
  mqttclient.setKeepAlive(5); // fast timeout, any slower will block the main loop too long
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(mqtt_callback);
}

void setup()
{
#if defined(debug) or defined(debug1)
  Serial.begin(74880);
  Serial.print("\nStarting Async_AutoConnect_ESP8266_minimal on " + String(ARDUINO_BOARD) + String("  "));
  // first get total memory before we do anything
  getFreeMemory();
#ifdef doubleResDet
  // double reset detect from start
  doubleResetDetect();
#endif
  pinMode(lampPin, OUTPUT);

#ifdef enableWifiManager
  Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);
#endif
#endif
  #ifdef wdtreset
  wdt_enable(WDTO_8S);
  wdt_reset();
  #endif

  Serial.print(F("Sketch free space: "));
  Serial.println(ESP.getFreeSketchSpace());
  pinMode(wificonfig_pin, INPUT); // pin for enter config
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, prgstatusrelay1WO); // 1 modul rozlaczony na pin 2-3 po przepieciu domyslnie uzyje 1-2 -przejscie przy braku aktywnosci
  digitalWrite(relay2, prgstatusrelay2CO); // 0 swieci zalaczony ssr relay2 woda relay1 co
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, 0);
  pinMode(gas_ain, INPUT);
  pinMode(ONE_WIRE_BUS, INPUT);

  display.begin();          // initializes the display
  display.setBacklight(90); // set the brightness to 100 %
  display.print("INIT");    // display INIT on the display
#ifdef debug
  Serial.println("display initialized...");
#endif

  if (loadConfig())
  {
    Serial.println(F("Config loaded:"));
    Serial.println(CONFIGURATION.version);
    Serial.println(CONFIGURATION.ssid);
    Serial.println(CONFIGURATION.pass);
    Serial.println(CONFIGURATION.mqtt_server);
  }
  else
  {
    Serial.println(F("Config not loaded!"));
    SaveConfig(); // overwrite with the default settings
  }
  allEnergy = energy1used + energy2used;

  Serial.println(("Connecting to " + String(ssid)));
  WiFi.hostname(String(me_lokalizacja).c_str());
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(String(me_lokalizacja).c_str());
  WiFi.begin(ssid, pass);

  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  WiFi.persistent(true);

  int deadCounter = 20;
  while (WiFi.status() != WL_CONNECTED && deadCounter-- > 0)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(("Failed to connect to " + String(ssid)));
    while (true);
  }
  else
  {
    Serial.println(F("ok"));
  }
  Serial.println(WiFi.getHostname());
  Serial.println(WiFi.localIP());

#ifdef enableWebSerial
  WebSerial.begin(&webserver);
  WebSerial.msgCallback(recvMsg);
#endif
  starting = false;
  setupMqtt();
  starthttpserver();

#ifdef ENABLE_INFLUX
  // InfluxDB
  InfluxClient.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);
  // Alternatively, set insecure connection to skip server certificate validation
  InfluxClient.setInsecure();
  // Add tags
  InfluxSensor.addTag("device", me_lokalizacja);
  // Check server connection
  if (InfluxClient.validateConnection())
  {
    sprintf(log_chars, "Connected to InfluxDB: %s", String(InfluxClient.getServerUrl()).c_str());
    log_message(log_chars);
  }
  else
  {
    sprintf(log_chars, "InfluxDB connection failed: %s", String(InfluxClient.getLastErrorMessage()).c_str());
    log_message(log_chars);
  }
#endif

  // Jako parametr mozemy podav dokladnosc - domyslnie 3
  // 0 - niski pobór energii - najszybszy pomiar
  // 1 - standardowy pomiar
  // 2 - wysoka precyzja
  // 3 - super wysoka precyzja - najwolniejszy pomiar

  ExistBM280 = bmp.begin();
  if (!ExistBM280)
  {
    log_message((char*)F("BM280 -Could not find a valid sensor, check wiring!"));
  }
  else
  {
    log_message((char*)F("BM280 initialized... OK"));
  };

  log_message((char*)F("18B20 begin"));
   // Init DS18B20 sensor
  sensors.begin();

  ads1.setGain(gain_resolution);
  ads1.setDataRate(RATE_ADS1115_475SPS);
  isadslinitialised = ads1.begin(0x48); // nie zaimplementowany wybor pin dla ads1115
  sprintf(log_chars,"ADS gain initialized...: %s", isadslinitialised?"OK":"MISSING");
  log_message(log_chars);
  Ro = MQCalibration(gas_ain);
  getCOGAS(gas_ain);
#ifdef debug
  Serial.println("gas ok...");
#endif

#ifdef debug
  Serial.println("end setup....");
  //    SaveConfig();
#endif
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.localIP());
  #ifdef enableArduinoOTA
  setupOTA();
  #endif
  MDNS.begin(me_lokalizacja.c_str());
  MDNS.addService("http", "tcp", 80);
  uptime = millis();
}

void getCOGAS(int pinpriv = gas_ain)
{
  if (millis() > (lastCOReadTime + avSensorReadTime*2) or lastCOReadTime == 0) {
    lastCOReadTime = millis();
    dcoval = MQGetPercentage(MQRead(pinpriv)/Ro,H2Curve);
    if (dcoval > 20000 or dcoval < 0) dcoval = 0;
  }
}

double getkWh(double EnergyAmpHourValue)
{
  const double kWat = 1000;
  return ((commonVolt * 1 * EnergyAmpHourValue * 1) / kWat);
}

void ReadEnergyUsed()
{
  if (((millis() - lastEnergyRead) > ReadEnergyTimeInterval or lastEnergyRead == 0) and isadslinitialised)
  {
    double lastEnergyAmpValue = 0;
    if (isnan(energy1used)) energy1used = 0;
    if (isnan(energy2used)) energy2used = 0;
    lastEnergyRead = millis();
    lastEnergyAmpValue = getenergy(ads_0);
    if (isnan(lastEnergyAmpValue)) lastEnergyAmpValue = 0;
    pump1energyLast = lastEnergyAmpValue;                 //to check if pump is running
    energy1used += getkWh(lastEnergyAmpValue);
    lastEnergyAmpValue = getenergy(ads_1);
    if (isnan(lastEnergyAmpValue)) lastEnergyAmpValue = 0;
    pump2energyLast = lastEnergyAmpValue;                   //to check if pump is running
    energy2used += getkWh(lastEnergyAmpValue);
    sprintf(log_chars,"energy readed: 1: %s  2: %s",String(energy1used,4).c_str(),String(energy2used,4).c_str());
    log_message(log_chars);
  }
}

void ReadTemperatures()
{
  if ((millis() - lastReadTimeTemps) > ReadTimeTemps or lastReadTimeTemps == 0)
  {
    lastReadTimeTemps = millis();
    bool assignedsensor = false;
    UnassignedTempSensor = "\0";
    String addrstr = "\0";
    // read temperatures

    sensors.setWaitForConversion(true); //
    sensors.requestTemperatures();      // Send the command to get temperatures
    // sensors.setWaitForConversion(false); // switch to async mode
    uint8_t addr[8];
    float temp1w;
    int count = sensors.getDS18Count();
    if (count == 0)
      count = sensors.getDeviceCount();
    sprintf(log_chars, "Reading 1wire 18B20 and other temps sensors... Count: %i", count);
    log_message(log_chars);
    if (count == 0)
      count = maxsensors;
    String temptmp = " Collecting 18B20 ROMS and temps:\n";
    for (int j = 0; j < count; j++)
    {
      temp1w = sensors.getTempCByIndex(j);
      if (temp1w == DS18B20nodata or temp1w == DS18B20nodata2 or temp1w == DS18B20nodata3 or temp1w == DS18B20nodata4 or temp1w == InitTemp)
        { temp1w = InitTemp; }
      addrstr = "";
      sensors.getAddress(addr, j);
      for (int i1 = 0; i1 < 8; i1++)
      {
      if (String(addr[i1], HEX).length() == 1) {
          addrstr += "0" + String(addr[i1], HEX);
      } else {
          addrstr += String(addr[i1], HEX); // konwersja HEX2StringHEX
      }}
      // zapisanie do zmiennej addr[8], addrstr, aktualiozacja index wskazany przez j i aktualozacja temp1w.
      addrstr.trim();
      addrstr.toUpperCase();
      sensors.setResolution(addr, 12);

      String addstrtmp = String(DallSens1_addr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (temp1w != InitTemp) coTherm = temp1w; assignedsensor = true; }
      addstrtmp = String(DallSens2_addr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (temp1w != InitTemp) waterTherm = temp1w; assignedsensor = true; }
      addstrtmp = String(DallSens3_addr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (temp1w != InitTemp) NTherm = temp1w; assignedsensor = true; }
      addstrtmp = String(DallSens4_addr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (temp1w != InitTemp) WTherm = temp1w; assignedsensor = true; }
      addstrtmp = String(DallSens5_addr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (temp1w != InitTemp) ETherm = temp1w; assignedsensor = true; }
      addstrtmp = String(DallSens6_addr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (temp1w != InitTemp) STherm = temp1w; assignedsensor = true; }

      if (!assignedsensor and UnassignedTempSensor.indexOf(addrstr) == -1 and temp1w != InitTemp) UnassignedTempSensor += String(addrstr) + " : " + String(temp1w) + "\n";  //
      assignedsensor = false;
      if (temp1w != InitTemp) temptmp += "       " + String(j) + ": 18B20 ROM= " + addrstr + ", temp: " + String(temp1w, 2) + "\n";
    }
    OutsideTempAvg = ((NTherm == InitTemp ? 0 : NTherm) + (ETherm == InitTemp ? 0 : ETherm) + (WTherm == InitTemp ? 0 : WTherm) + (STherm == InitTemp ? 0 : STherm)) / ((NTherm == InitTemp ? 0 : 1) + (ETherm == InitTemp ? 0 : 1) + (WTherm == InitTemp ? 0 : 1) + (STherm == InitTemp ? 0 : 1));
    if (isnan(OutsideTempAvg)) OutsideTempAvg = InitTemp;
    log_message((char *)temptmp.c_str());
    UnassignedTempSensor.trim();
    if (UnassignedTempSensor != "")
    {
      sprintf(log_chars, "Unassigned Sensors: %s and AvgOutside Temp: %s", UnassignedTempSensor.c_str(), String(OutsideTempAvg,2).c_str());
      log_message(log_chars);
    }

  if (ExistBM280)
  {
    bmTemp = bmp.readTemperature();
    if (isnan(bmTemp)) bmTemp = InitTemp;
    dbmpressval = bmp.readPressure() / 100;
    if (millis() % 50000 == 0)
    {
      // Obliczamy wysokosc dla domyslnego cisnienia przy pozimie morza
      // p0 = 1013.25 millibar = 101325 Pascal
      bm_high = bmp.readAltitude();
      // Jesli znamy aktualne cisnienie przy poziomie morza,
      // mozemy dokladniej wyliczyc wysokosc, padajac je jako parametr
      bm_high_real = bmp.readAltitude(102520);
    }
  }

    //  dcoval

#ifdef enableDHT
    dht.read(true);
    delay(200);
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(h) or isnan(t))
    {
      log_message((char *)F("Failed to read from DHT sensor!"));
      // if (isnan(t)) t=InitTemp;
      // if (isnan(h)) h=0;
    }
    else
    {
      dhtreadtime = millis();
    }

    sprintf(log_chars, "Get DHT values t= %s, humid: %s", String(t, 2), String(h, 1));
    log_message(log_chars);
    if (!isnan(h))
      humiditycor = h;
    if (!isnan(t))
      tempcor = t; // to check ds18b20 also ;)
#endif
  }
}

void displayCoCo()
{
  display.print("COCO");
  display.blink();
}

void loop()
{
  counter++;
  check_wifi();
  #ifdef enableArduinoOTA
  // Handle OTA first.
  ArduinoOTA.handle();
  #endif
  ReadTemperatures();
  ReadEnergyUsed();

  mqttclient.loop();
  #ifdef wdtreset
  wdt_reset();
  #endif
#ifdef enableWifiManager
  if (initialConfig == true)
  {
#if defined(debug) or defined(debug1)
    Serial.println(F("On demand config..."));
#endif
    // webserver.end();
    // WiFi.disconnect();

    count_nowifi = 0;
    ondemandwifiCallback();
  };
#endif

  bool C_W_read = digitalRead(wificonfig_pin);
  {
    if (C_W_read != last_C_W_state)
    {
      time_last_C_W_change = millis();
    }
    if ((millis() - time_last_C_W_change) > (long unsigned int)1000)
    {
      if (C_W_read != C_W_state)
      {
        displayCoCo();
        najpierwCO = true;
        waitCOStartingmargin = millis();
      }
    }
    if ((millis() - time_last_C_W_change) > (long unsigned int)C_W_delay)
    {
      if (C_W_read != C_W_state)
      {
#ifdef debug
        Serial.println("Triger state changed");
#endif
        C_W_state = C_W_read;
        if (C_W_state == HIGH)
        {
#ifdef debug
          Serial.println(F("Enter ondemand config"));
#endif
          display.blink();
// webserver.end();
#ifdef enableWifiManager
          ondemandwifiCallback();
#endif
          starting = true;
        };
      };
    };
    last_C_W_state = C_W_read;
  };

  if ((unsigned long)(millis() - lastloopRunTime) > (LOOP_WAITTIME))
  {
    lastloopRunTime = millis();
    // check mqtt
    if ((WiFi.isConnected()) && (!mqttclient.connected()))
    {
      log_message((char *)"Lost MQTT connection!");
      mqtt_reconnect();
    }
    // log stats
    //    #include "configmqtttopics.h"
    String message = F("stats: Uptime: ");
    message += uptimedana();
    message += F(" ## Free memory: ");
    message += getFreeMemory();
    message += F("% ");
    message += ESP.getFreeHeap();
    message += F(" bytes ## Wifi: ");
    message += getWifiQuality();
    message += F("% ## Mqtt reconnects: ");
    message += mqttReconnects;
//    message += F("%");
    log_message((char *)message.c_str());

    String stats = F("{\"uptime\":");
    stats += String(millis());
    stats += F(",\"version\":");
    stats += me_version;
    stats += F(",\"voltage\":");
    stats += 0;
    stats += F(",\"free memory\":");
    stats += getFreeMemory();
    stats += F(",\"ESP cyclecount\":");
    stats += ESP.getCycleCount();
    stats += F(",\"wifi\":");
    stats += getWifiQuality();
    stats += F(",\"mqtt reconnects\":");
    stats += mqttReconnects;
    stats += F("}");
    mqttclient.publish(STATS_TOPIC.c_str(), stats.c_str(), mqtt_Retain);

    // get new data
    //  if (!heishamonSettings.listenonly) send_panasonic_query();

    // Make sure the LWT is set to Online, even if the broker have marked it dead.
    mqttclient.publish(WILL_TOPIC.c_str(), "Online");
    getCOGAS(gas_ain);

    if (WiFi.isConnected())
    {
      //      MDNS.announce();
    }
  }

  if (shouldSaveConfig == true)
  {
#if defined(debug) or defined(debug1)
    Serial.println("SAVING CONF");
#endif
    SaveConfig();
#if defined(debug) or defined(debug1)
    Serial.println("SAVING ENEREGY");
#endif
    SaveEnergy();
#ifdef debug
    Serial.println("saved en...");
#endif
    // WiFi.mode(WIFI_STA);
    //    WiFi.forceSleepBegin();
    //    webserver.end();
    //    delay(5000);
    //    WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while (1)ESP.restart(); wdt_reset(); ESP.restart();
  }

  if (((millis() - lastUpdatemqtt) > mqttUpdateInterval_ms) or lastUpdatemqtt == 0 or receivedmqttdata == true) // recived data ronbi co 800ms -wylacze ten sttus dla odebrania news
  {
    sprintf(log_chars, "mqtt+influxDB lastUpdatemqtt: %s receivedmqttdata: %s mqttUpdateInterval_ms: %s", String(lastUpdatemqtt).c_str(), String(receivedmqttdata).c_str(), String(mqttUpdateInterval_ms).c_str());
    log_message(log_chars);
    receivedmqttdata = false;
    lastUpdatemqtt = millis();
    updateMQTTData();
#ifdef ENABLE_INFLUX
    updateInfluxDB(); // i have on same server mqtt and influx so when mqtt is down influx probably also ;(  This   if (InfluxClient.isConnected())  doesn't work forme 202205
#endif
  }

  if ((millis() - savetime > (saveminut)) and (allEnergy != (energy1used + energy2used)))
  {
    SaveEnergy();
    sprintf(log_chars, "Saved energy after savetime, allEn: %s en1: %s en2: %s", String(allEnergy).c_str(), String(energy1used).c_str(), String(energy2used).c_str());
    log_message(log_chars);
    allEnergy = energy1used + energy2used;
    savetime = millis();
  }
  if ((panicbuz == true) and (counter % 150 == 0))
  {
    digitalWrite(buzzer, !digitalRead(buzzer));
  }

  if (counter % (30 * 1000) == 0)
  {
    // odczyt czujnika CO
    log_message((char *)F("Check gas leak"));
    gas_leak_check();
    // check_temps_pumps();
  }

  if (counter % (10 * last_case + what_display) * 2.3 == 0 or firstrun == true)
  { // 4casy
    firstrun = false;
    display_temp_rotation();
  }

  if (counter % 100 == 0)
  {
    Serial.println("check_temps_pumps();");
    check_temps_pumps();
  }
}

void ChangeRelayStatus(uint8_t numer, bool stan)
{
  // LOW wlacza  HIGH wylacza
  sprintf_P(log_chars, "ChangeRelayStatus. pompa: %s, Stan: %s, prgstatusrelay1WO: %s, prgstatusrelay2CO: %s", (numer == pumpWaterRelay ? "WODA" : "CO"), stan ? "ON" : "OFF", prgstatusrelay1WO ? "ON" : "OFF", prgstatusrelay2CO ? "ON" : "OFF");
  log_message(log_chars);
  if (numer == pumpWaterRelay)
  {
    digitalWrite(relay1, !stan); // LOW wlacza  HIGH wylacza
    //   if (stan == HIGH) {relay1S.turnOn();} else {relay1S.turnOff();}  //WYLACZ ZMIENILEM LOGIKE
    prgstatusrelay1WO = stan;
  }
  if (numer == pumpCoRelay)
  {
    digitalWrite(relay2, !stan); // LOW wlacza  HIGH wylacza
    //   if (stan == HIGH)  {relay2S.turnOn();} else {relay2S.turnOff();}   //WYLACZ ZMIENILEM LOGIKE
    prgstatusrelay2CO = stan;
  }
}

void check_temps_pumps()
{
  //  coTherm = coThermometerS->getlast();
  //  waterTherm = waterThermometerS->getlast();

  //  if (sensorses < 2 or coTherm < 0 or waterTherm < 0) {
  //    display.print("Err");
  //    digitalWrite(relay2, 0); digitalWrite(relay1, 0);
  //    delay(20000);
  //      resetFunc();
  //  }

  sprintf_P(log_chars, "CheckPumps: coTherm+histereza > Watertemp: %s, coTherm: %s, watertherm+histereza: %s", String(coTherm + histereza > waterTherm).c_str(), String(coTherm).c_str(), String(waterTherm + histereza).c_str());
  log_message(log_chars);
  if (isnan(coTherm) or isnan(waterTherm) or coTherm >= panic or coTherm < 1)
  {
    // gdy zawioda czujniki lub panika -wlacz pompy
    // LOW wlacza  HIGH wylacza
    // LOW wlacza  HIGH wylacza
    // relay1 woda
    // relay2 CO
    prgstatusrelay1WO = HIGH;
    prgstatusrelay2CO = HIGH;
    log_message((char*)F("Force to panic... Change state relays to ON..."));
    panicbuz = true;
  }
  else
  ChangeRelayStatus(pumpWaterRelay, prgstatusrelay1WO);
  ChangeRelayStatus(pumpCoRelay, prgstatusrelay2CO);
}

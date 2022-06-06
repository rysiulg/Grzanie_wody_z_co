// Sterowanie grzeniem wody z co
//Arduino MiniPro->ESP8266 v.2.7.1 arduino 1.8.12 i zmodyfikowany Filesystem do 3MBSketchOTA/SPIFF1MB dla NodeMCU1.0 (ESP 12E) !!!!!!!!!!!!!!!!!!!!!!!!
//v.1.0 20190922start
//v.1.1 -modyfikacja przekaznika 20190923 -przepiecie by dzialal domyślnie aktywnie bez zasilania oraz odłączanie pompy do wody poprzez aktywacje przekaznika
//v.2 -modyfikacja do ESP8266 Wifi+Supla+http update+SSR zamiast przekaznika
//v2.02 modyfikacje dla supla 2.3 i enchanced config +czujnik co i bme085 (cisnienie i temp)
//v2.03 modyfikacje zmiana interfejsu www
//v2.05 obsluga licznika energii z zapisaniem np. co 15 minut w SPIFF i obsluga definicji SN 18B20 z config -nie startowalo jeszcze gdy zlapalo wifi...
//v2.10 powrot do ESP 2.6.1 httpupdater robil crash po aktualizacji w wersji 2.7
//2.11 spiecie update menager i async web
//v2.12 skorygowano wylaczanie/wlaczanie pomp i dodany link uptime
//v2.13 kalibracja gas sensor
//v2.14 dopisane force by bytton (zuzycie energii)
//v2.15 wdresety, ustawienie wymuszania pomp w zaleznosci od przycisku i dodatkowo gdy temp na zewnatrz srednia z czujnikow < forceCObelow
//v2.15a-i drobne poprawki logiki
//v2.16 -buczalo gdy temp co byla <15 -zmiana na <1, zmniejszona wartosc forceCObelow do 7st
//v3.00 -move SUPLA to MQTT, upgrade Json 5->6, enable stack protection in esp config, arduino po aktualizacji traktuje string="" jako 0.0000 i przez to autoryzacja sie wysypuje w webhandler
//v3.01 -drobne poprawki, wywalone z bliblioteki ESPAsyncWebServer linijek by usunac kom. o autoryzacjach  w WebHandlerImpl.h //    if((_username != "" && _password != "") && !request->authenticate(_username.c_str(), _password.c_str()))  return request->requestAuthentication();//44444444444444444444444444444444444444444446666666666666666666666666666666666666666666666666666666666
//v3.02 -uruchomione z config na guziku
//v3.02a -aktualizacja ikonek mqtt, dodany prefix mqtt oraz uzupelnione opcje device w mqtt
//v3.02b -dolozony parametr opozniajacy o 1 godz. wylaczenie wymuszenia zmiany priotytetu pompy z woda na Co gdy temp < od temp aktywacji pomp
//  Copyright (C) MARM.pl
//v3.02e -poprawa display i dodany dwukropek do temps
//v202205 remove arduinojson, HomeassistantArduino -implement own


//*************************************************
//pins



//#include "SPIFFS.h"
//#include <LittleFS.h>
//#define SPIFFS LITTLEFS




//*************************************************0x28, 0xFF, 0x4B, 0x72, 0x80, 0x14, 0x02, 0x77
#include "main.h"

#include "ClassesAndFunctions.h"



Sensor::OnePhaseElectricityMeter* pump1energyS;
Sensor::OnePhaseElectricityMeter* pump2energyS;
Sensor::GasGeneralPurposeMeasurementBase* gasCOMeterS;

HAM::Sensor::DS18B20 *coThermometerS;// (dcoThermstat); // "temp" is unique ID of the sensor. You should define your own ID.
HAM::Sensor::DS18B20 *waterThermometerS;// (dwaterThermstat); // "temp" is unique ID of the sensor. You should define your own ID.
HAM::Sensor::DS18B20 *NThermometerS;// (dNThermometerS); // "temp" is unique ID of the sensor. You should define your own ID.
HAM::Sensor::DS18B20 *WThermometerS;// (dWThermometerS); // "temp" is unique ID of the sensor. You should define your own ID.
HAM::Sensor::DS18B20 *EThermometerS;// (dEThermometerS); // "temp" is unique ID of the sensor. You should define your own ID.
HAM::Sensor::DS18B20 *SThermometerS;// (dSThermometerS); // "temp" is unique ID of the sensor. You should define your own ID.
HAM::Sensor::DS18B20 *T1ThermometerS;// (dT1ThermometerS); // "temp" is unique ID of the sensor. You should define your own ID.


//*************************************************

#include "funkcje.h"
#include "configPortal.h"

//*************************************************

void setup() {
  #if defined (debug) or defined (debug1)
  Serial.begin(74880);
  Serial.print("\nStarting Async_AutoConnect_ESP8266_minimal on " + String(ARDUINO_BOARD)+String("  "));
  #ifdef enableWifiManager
  Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);
  #endif
  #endif

  wdt_enable(WDTO_8S);
  wdt_reset();
  delay(200);
  String tmp;
  tmp = dcoThermstat;
  tmp.toCharArray(sts[0].sname, tmp.length()+1); //+1 bo obcinalo koncowki, sizeof obcina dodatkowo o 1 znak -jak poprawia cos w Stringach to moze robic bledy
  #ifdef debug
    Serial.print((String)sts[0].sname);
    Serial.print(" = ");
    Serial.println(tmp);
  #endif
  tmp = dwaterThermstat;
  tmp.toCharArray(sts[1].sname, tmp.length()+1);
  #ifdef debug
    Serial.print((String)sts[1].sname);
    Serial.print(" = ");
    Serial.println(tmp);
  #endif
  tmp = dNThermometerS;
  tmp.toCharArray(sts[2].sname, tmp.length()+1);
  #ifdef debug
    Serial.print((String)sts[2].sname);
    Serial.print(" = ");
    Serial.println(tmp);
  #endif
  tmp = dWThermometerS;
  tmp.toCharArray(sts[3].sname, tmp.length()+1);
  #ifdef debug
    Serial.print((String)sts[3].sname);
    Serial.print(" = ");
    Serial.println(tmp);
  #endif
  tmp = dEThermometerS;
  tmp.toCharArray(sts[4].sname, tmp.length()+1);
  #ifdef debug
    Serial.print((String)sts[4].sname);
    Serial.print(" = ");
    Serial.println(tmp);
  #endif
  tmp = dSThermometerS;
  tmp.toCharArray(sts[5].sname, tmp.length()+1);
  #ifdef debug
    Serial.print((String)sts[5].sname);
    Serial.print(" = ");
    Serial.println(tmp);
  #endif
#ifdef newSensorT1
  tmp = dT1ThermometerS;
  tmp.toCharArray(sts[6].sname, tmp.length()+1);
#endif


  Serial.print(F("Sketch free space: "));
  Serial.println(ESP.getFreeSketchSpace());
  pinMode(wificonfig_pin, INPUT); //pin for enter config
  pinMode (relay1, OUTPUT);
  pinMode (relay2, OUTPUT);
  digitalWrite(relay1, prgstatusrelay1WO); // 1 modul rozlaczony na pin 2-3 po przepieciu domyslnie uzyje 1-2 -przejscie przy braku aktywnosci
  digitalWrite(relay2, prgstatusrelay2CO);  //0 swieci zalaczony ssr relay2 woda relay1 co
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, 0);
  pinMode(gas_ain, INPUT);
  pinMode(ONE_WIRE_BUS, INPUT);

  display.begin();            // initializes the display
  display.setBacklight(90);  // set the brightness to 100 %
  display.print("INIT");      // display INIT on the display
  #ifdef debug
    Serial.println("display initialized...");
  #endif


  for (uint8_t x = 0; x < 8; x++) { //Wpisz adresy bazowe czujnikow ktore znam
    uint8_t tmphex;
    tmphex = nibble(coThermometer[x * 2]) * 16 + nibble(coThermometer[x * 2 + 1]);
    if ((sts[0].ssn[x]) == 0) sts[0].ssn[x] = tmphex;
    tmphex = nibble(waterThermometer[x * 2]) * 16 + nibble(waterThermometer[x * 2 + 1]);
    if ((sts[1].ssn[x]) == 0) sts[1].ssn[x] = tmphex;

    tmphex = nibble(sens2[x * 2]) * 16 + nibble(sens2[x * 2 + 1]);
    if ((sts[2].ssn[x]) == 0) sts[2].ssn[x] = tmphex;
    tmphex = nibble(sens3[x * 2]) * 16 + nibble(sens3[x * 2 + 1]);
    if ((sts[3].ssn[x]) == 0) sts[3].ssn[x] = tmphex;
    tmphex = nibble(sens4[x * 2]) * 16 + nibble(sens4[x * 2 + 1]);
    if ((sts[4].ssn[x]) == 0) sts[4].ssn[x] = tmphex;
    tmphex = nibble(sens5[x * 2]) * 16 + nibble(sens5[x * 2 + 1]);
    if ((sts[5].ssn[x]) == 0) sts[5].ssn[x] = tmphex;

  }

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
  allEnergy = energy1used+energy2used;

  #ifdef debug
    Serial.println(F("Assigned 18B20 serial numbers:"));
  dumpByteArray(sts[0].ssn);
  dumpByteArray(sts[1].ssn);
  dumpByteArray(sts[2].ssn);
  dumpByteArray(sts[3].ssn);
  dumpByteArray(sts[4].ssn);
  dumpByteArray(sts[5].ssn);
#ifdef newSensorT1
  dumpByteArray(sts[6].ssn);
#endif
  #endif

  Serial.println(("Connecting to " + String(ssid)));
  WiFi.mode(WIFI_STA);
  WiFi.hostname(String(me_lokalizacja).c_str());
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  WiFi.persistent(true);
  WiFi.begin(ssid, pass);

  int deadCounter = 20;
  while (WiFi.status() != WL_CONNECTED && deadCounter-- > 0)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(("Failed to connect to " + String(ssid)));
    while (true)
      ;
  }
  else
  {
    Serial.println(F("ok"));
  }

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);

  pump1energyS = new Sensor::OnePhaseElectricityMeter((char*)String(ads_0).c_str());
  pump2energyS = new Sensor::OnePhaseElectricityMeter((char*)String(ads_1).c_str()); //Sensor::OnePhaseElectricityMeter *pump2energyS = new Sensor::OnePhaseElectricityMeter(ads_1,"Piwnica-piec pump2energyS temperature");
  coThermometerS = new HAM::Sensor::DS18B20(ONE_WIRE_BUS,  String(dcoThermstat),sts[0].ssn); //, coThermometer);    // 4
  waterThermometerS = new HAM::Sensor::DS18B20(ONE_WIRE_BUS, String(dwaterThermstat), sts[1].ssn); //, waterThermometer);
  NThermometerS = new HAM::Sensor::DS18B20(ONE_WIRE_BUS, String(dNThermometerS), sts[2].ssn);
  WThermometerS = new HAM::Sensor::DS18B20(ONE_WIRE_BUS, String(dWThermometerS), sts[3].ssn);
  EThermometerS = new HAM::Sensor::DS18B20(ONE_WIRE_BUS, String(dEThermometerS), sts[4].ssn);
  SThermometerS = new HAM::Sensor::DS18B20(ONE_WIRE_BUS, String(dSThermometerS), sts[5].ssn);
  #ifdef newSensorT1
  T1ThermometerS = new HAM::Sensor::DS18B20(ONE_WIRE_BUS, String(dT1ThermometerS), sts[6].ssn);
  #endif
  gasCOMeterS = new Sensor::GasGeneralPurposeMeasurementBase(gas_ain); //gas_sensor

  // Jako parametr mozemy podav dokladnosc - domyslnie 3
  // 0 - niski pobór energii - najszybszy pomiar
  // 1 - standardowy pomiar
  // 2 - wysoka precyzja
  // 3 - super wysoka precyzja - najwolniejszy pomiar

  ExistBM280=bmp.begin();
  if (!ExistBM280) {
    #if defined (debug) or defined (debug1)
      Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    #endif
    //while (1);
  } else {
    #ifdef debug
      Serial.println(F("bme initialized..."));
    #endif
    bm_high = bmp.readAltitude();
    // Jesli znamy aktualne cisnienie przy poziomie morza,
    // mozemy dokladniej wyliczyc wysokosc, padajac je jako parametr
    bm_high_real = bmp.readAltitude(102520);
    //bmAttitudeS.setDeviceClass("None");
    dbmpressval=bmp.readPressure() / 100;
    //dodaj do HA bmAttitude
  };

  ads1.setGain(gain_resolution);
  ads1.setDataRate(RATE_ADS1115_475SPS);
  #if defined (debug) or defined (debug1)
    Serial.println(F("ads gain initialized..."));
  #endif
  isadslinitialised=ads1.begin(0x48);  //nie zaimplementowany wybor pin dla ads1115
  Ro = MQCalibration(gas_ain);
  #if defined (debug) or defined (debug1)
  if (isadslinitialised) Serial.println(F("ads initialized..."));
  #endif

  pump1energyS->onInit();
  #ifdef debug
    Serial.println(F("1st OPEM ok..."));
  #endif
  #ifdef debug
    Serial.println(F("2nd OPEM before onInit ok..."));
  #endif
  pump2energyS->onInit();
  #ifdef debug
    Serial.println(F("2nd OPEM ok..."));
  #endif

  gasCOMeterS->Init();
  #ifdef debug
    Serial.println("gas ok...");
  #endif

  wdt_reset();
  #ifdef debug
    Serial.println("begin 18B20 1st coTherm OK...");
  #endif
//  AssignSensors();
  coThermometerS->Init();
  waterThermometerS->Init();
  NThermometerS->Init();
  WThermometerS->Init();
  EThermometerS->Init();
  SThermometerS->Init();
  #ifdef newSensorT1
  T1ThermometerS->Init();
  #endif
  #ifdef enableWebSerial
  WebSerial.begin(&webserver);
  WebSerial.msgCallback(recvMsg);
  #endif

  #ifdef ENABLE_INFLUX
  //InfluxDB
  InfluxClient.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);
  // Alternatively, set insecure connection to skip server certificate validation
  InfluxClient.setInsecure();
  // Add tags
  InfluxSensor.addTag("device", me_lokalizacja);
    // Check server connection
    if (InfluxClient.validateConnection()) {
        #ifdef enableWebSerial
        WebSerial.print(String(millis())+": "+"Connected to InfluxDB: ");
        WebSerial.println(InfluxClient.getServerUrl());
        #endif
        #ifdef debug
        Serial.print(String(millis())+": "+"Connected to InfluxDB: ");
        Serial.println(InfluxClient.getServerUrl());
        #endif
    } else {
      #ifdef enableWebSerial
      WebSerial.print(String(millis())+": "+"InfluxDB connection failed: ");
      WebSerial.println(InfluxClient.getLastErrorMessage());
      #endif
      #ifdef debug
      Serial.print(String(millis())+": "+"InfluxDB connection failed: ");
      Serial.println(InfluxClient.getLastErrorMessage());
      #endif
    }
  #endif

  #ifdef debug
    Serial.println("end setup....");
//    SaveConfig();
  #endif
  uptime=millis();
}

void displayCoCo()
{
  display.print("COCO");
  display.blink();
}

void loop() {
  wdt_reset();
  counter++;

#ifdef enableWifiManager
  if (initialConfig == true) {
    #if defined (debug) or defined (debug1)
      Serial.println(F("On demand config..."));
    #endif
   // webserver.end();
   // WiFi.disconnect();

    count_nowifi=0;
    ondemandwifiCallback();
  };
#endif

  bool C_W_read = digitalRead(wificonfig_pin); {
    if (C_W_read != last_C_W_state) {
      time_last_C_W_change = millis();
    }
    if ((millis() - time_last_C_W_change) > (long unsigned int)1000) {
      if (C_W_read != C_W_state) {
        displayCoCo();
        najpierwCO=true;
        waitCOStartingmargin = millis();
      }
    }
    if ((millis() - time_last_C_W_change) > (long unsigned int)C_W_delay) {
      if (C_W_read != C_W_state) {
        #ifdef debug
        Serial.println("Triger state changed");
        #endif
        C_W_state = C_W_read;
        if (C_W_state == HIGH) {
          #ifdef debug
            Serial.println(F("Enter ondemand config"));
          #endif
          display.blink();
          //webserver.end();
          #ifdef enableWifiManager
          ondemandwifiCallback() ;
          #endif
          starting = true;
        };
      };
    };
    last_C_W_state = C_W_read;
  };

  if (shouldSaveConfig == true) {
    #if defined (debug) or defined (debug1)
      Serial.println("SAVING CONF");
    #endif
    SaveConfig();
    #if defined (debug) or defined (debug1)
      Serial.println("SAVING ENEREGY");
    #endif
    SaveEnergy();
    #ifdef debug
      Serial.println("saved en...");
    #endif
    //WiFi.mode(WIFI_STA);
    WiFi.forceSleepBegin();
    webserver.end();
    delay(5000);
    WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while (1)ESP.restart(); wdt_reset(); ESP.restart();
  }

  //unsigned long now = millis() + 0; // TO AVOID compare -2>10000 which is true ??? why?
  // check mqtt is available and connected in other case check values in api.
  // if (mqtt_offline_retrycount == mqtt_offline_retries)
  // {
  //   if ((millis() - lastmqtt_reconnect) > mqtt_offline_reconnect_after_ms)
  //   {
  //     lastmqtt_reconnect = millis();
  //     mqtt_offline_retrycount = 0;
  //     #ifdef debug
  //     Serial.println(String(millis())+": "+F("MQTT connection problem -now reset retry counter and try again..."));
  //     #endif
  //     #ifdef enableWebSerial
  //     WebSerial.println(String(millis())+": "+F("MQTT connection problem -now reset retry counter and try again..."));
  //     #endif
  //   }
  //   else
  //   {
  //     #ifdef debug
  //     Serial.println(F("MQTT connection problem -now try get temp data alternative way (room temp and NEWS temp and Carbon CO Water pump status")); //Insert some delay to limit messages to webserial or errors
  //     #endif
  //     delay(1500);
  //     // best place to function get values from http when mqtt is unavailable
  //     //lastNEWSSet = now; // reset counter news temp with alternative parse value way
  //     //temp_NEWS_count = 0;
  //   }
  // }
  // else
  // {

  // }


  if (WiFi.status() == WL_CONNECTED  and WiFi.localIP() != IPAddress(0, 0, 0, 0) and initialConfig == false) {
    if (starting) {
      // webserver.end();
      // webserver.reset();

      DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
      DefaultHeaders::Instance().addHeader("Server",me_lokalizacja);
      DefaultHeaders::Instance().addHeader("Title",me_lokalizacja);

      subWebServers(); //update /gt /
      webserver.begin();//httpwebserver.begin();
      reconnect();  //subscribe to mqtt
      starting = false;
      count_nowifi=0;
      #if defined (debug) or defined (debug1)
        Serial.println("");
        Serial.println("CONNECTED");
        Serial.print("local IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("subnetMask: ");
        Serial.println(WiFi.subnetMask());
        Serial.print("gatewayIP: ");
        Serial.println(WiFi.gatewayIP());
        long rssi = WiFi.RSSI();
        Serial.print("Signal Strength (RSSI): ");
        Serial.print(rssi);
        Serial.println(" dBm");
      #endif
    }
    //Ethernet.maintain();
    #ifdef debug
//      Serial.println(F("Loop iterate starts"));
    #endif
    ReadTemperatures();
    coThermometerS->iterateAlways();
    waterThermometerS->iterateAlways();
    NThermometerS->iterateAlways();
    WThermometerS->iterateAlways();
    EThermometerS->iterateAlways();
    SThermometerS->iterateAlways();
    #ifdef newSensorT1
    T1ThermometerS->iterateAlways();
    #endif
    if (ExistBM280) {
      bmTemp = bmp.readTemperature();
      dbmpressval = bmp.readPressure() / 100;
      if (counter % 50000 == 0) {
        // Obliczamy wysokosc dla domyslnego cisnienia przy pozimie morza
        // p0 = 1013.25 millibar = 101325 Pascal
        bm_high = bmp.readAltitude();
        // Jesli znamy aktualne cisnienie przy poziomie morza,
        // mozemy dokladniej wyliczyc wysokosc, padajac je jako parametr
        bm_high_real = bmp.readAltitude(102520);
      }
    }
    pump1energyS->iterateAlways();
    pump2energyS->iterateAlways();
    gasCOMeterS->iterateAlways();
    OutsideTempAvg = (EThermometerS->getlast() +  WThermometerS->getlast() + SThermometerS->getlast() + NThermometerS->getlast()) / 4;// * 0;   //SThermometerS->getValue()  /4 -nie mam S wiec bez tego
    if (!mqttClient.connected())
    {
      #ifdef debug
      Serial.println(String(millis())+": "+F("MQTT connection problem -try to connect again..."));
      #endif
      #ifdef enableWebSerial
      WebSerial.print(String(millis())+": "+F("MQTT connection problem -try to connect again...    -"));
      #endif
      delay(2000);
      reconnect();
    }
    else
    {
      mqttClient.loop();
    }

    if (((millis() - lastUpdatemqtt) > mqttUpdateInterval_ms)  or lastUpdatemqtt==0 or receivedmqttdata == true)  //recived data ronbi co 800ms -wylacze ten sttus dla odebrania news
    {
      #ifdef debug1
      Serial.println(String(millis())+" mqtt+influ "+"lastUpdatemqtt: "+String(lastUpdatemqtt)+" receivedmqttdata: "+String(receivedmqttdata)+" mqttUpdateInterval_ms: "+String(mqttUpdateInterval_ms));
      Serial.println(String(lastUpdatemqtt)+": Update MQTT and InfluxDB data: ");
      #endif
      #ifdef enableWebSerial
      WebSerial.println(String(millis())+" mqtt+influ "+"lastUpdatemqtt: "+String(lastUpdatemqtt)+" receivedmqttdata: "+String(receivedmqttdata)+" mqttUpdateInterval_ms: "+String(mqttUpdateInterval_ms));
      WebSerial.println(String(lastUpdatemqtt)+": Update MQTT and InfluxDB data: ");
      #endif
      receivedmqttdata = false;
      lastUpdatemqtt = millis();
      if (espClient.connected()) {
        updateMQTTData();
        #ifdef ENABLE_INFLUX
        updateInfluxDB(); //i have on same server mqtt and influx so when mqtt is down influx probably also ;(  This   if (InfluxClient.isConnected())  doesn't work forme 202205
        #endif
      }

    }
//    MDNS.update();
    #ifdef debug
//      Serial.println(F("Loop iterate ends"));
    #endif
  } else {
    #ifdef debug
      Serial.print(F("Starting again Wifi: "));
      Serial.println(count_nowifi);
    #endif
    count_nowifi += 1;
    if (count_nowifi > 20000) initialConfig = true;
    Serial.print(String(millis()));
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
  }

  if ((millis() - savetime > (saveminut)) and (allEnergy != (energy1used + energy2used))) {
    SaveEnergy();
    #if defined (debug) or defined (debug1)
      Serial.println("Saved energy after savetime");
      Serial.println("allEn: "+String(allEnergy)+" en1: "+String(energy1used)+" en2: "+String(energy2used));
    #endif
    allEnergy = energy1used + energy2used;
    savetime = millis();
  }
  if ((panicbuz == true) and (counter % 150 == 0)) {
    digitalWrite(buzzer, !digitalRead(buzzer));
  }

  if (counter % (30 * 1000) == 0 ) {
    //odczyt czujnika CO
    #if defined(debug) or defined (debug1)
      Serial.println(F("Check gas leak"));
    #endif
    #if defined(enableWebSerial)
      WebSerial.println(String(millis())+F(": Check gas leak"));
    #endif
    gas_leak_check();
  //check_temps_pumps();

  }
  if (counter % (10 * last_case + what_display )*2.3 == 0 or firstrun == true) { //4casy
    firstrun = false;
    display_temp_rotation();
  }

  if (counter % 100 == 0) {
    check_temps_pumps();
    if (waterThermometerS->getlast()==TEMPERATURE_NOT_AVAILABLE) waterTherm=waterThermometerS->getValue();
    waterTherm=waterThermometerS->getlast();
    if (coThermometerS->getlast()==TEMPERATURE_NOT_AVAILABLE) coTherm=coThermometerS->getValue();
    coTherm=coThermometerS->getlast();
    if (NThermometerS->getlast()==TEMPERATURE_NOT_AVAILABLE) NThermometerS->getValue();
    if (SThermometerS->getlast()==TEMPERATURE_NOT_AVAILABLE) SThermometerS->getValue();
    if (WThermometerS->getlast()==TEMPERATURE_NOT_AVAILABLE) WThermometerS->getValue();
    if (EThermometerS->getlast()==TEMPERATURE_NOT_AVAILABLE) EThermometerS->getValue();
    #ifdef newSensorT1
    if (T1ThermometerS->getlast()==TEMPERATURE_NOT_AVAILABLE) T1ThermometerS->getValue();
    #endif
    supla_and_relay_obsluga(pumpWaterRelay,prgstatusrelay1WO);  //faked numer dla aktualizacji statusu
    supla_and_relay_obsluga(pumpCoRelay,prgstatusrelay2CO);
  }
}

void supla_and_relay_obsluga(uint8_t numer, bool stan) {
  //LOW wlacza  HIGH wylacza
  #ifdef debug2
    Serial.print("Supla_and_relay_obsluga. numer: ");
    Serial.print(numer);
    Serial.print("   Stan: ");
    Serial.print(stan);
    Serial.print(" prgstatusrelay1WO: ");
    Serial.print(prgstatusrelay1WO);
    Serial.print(" prgstatusrelay1WO: ");
    Serial.println(prgstatusrelay1WO);
  #endif
  #ifdef enableWebSerial
    WebSerial.print(String(millis())+F(": Supla_and_relay_obsluga. pompa: "));
    WebSerial.print(String(numer == pumpWaterRelay ? "WODA":"CO"));
    WebSerial.print(F("   Stan: "));
    WebSerial.print(stan);
    WebSerial.print(F(" prgstatusrelay1WO: "));
    WebSerial.print(prgstatusrelay1WO);
    WebSerial.print(F(" prgstatusrelay1WO: "));
    WebSerial.print(prgstatusrelay1WO);
    WebSerial.print(F(" Counter: "));
    WebSerial.println(String(counter));
  #endif
  if (numer == pumpWaterRelay) {
    digitalWrite(relay1, !stan);  //LOW wlacza  HIGH wylacza
 //   if (stan == HIGH) {relay1S.turnOn();} else {relay1S.turnOff();}  //WYLACZ ZMIENILEM LOGIKE
    prgstatusrelay1WO = stan;
  }
  if (numer == pumpCoRelay) {
    digitalWrite(relay2, !stan);  //LOW wlacza  HIGH wylacza
 //   if (stan == HIGH)  {relay2S.turnOn();} else {relay2S.turnOff();}   //WYLACZ ZMIENILEM LOGIKE
    prgstatusrelay2CO = stan;
  }
}



void check_temps_pumps() {
  //  coTherm = coThermometerS->getlast();
  //  waterTherm = waterThermometerS->getlast();
  double pump1energyLast = pump1energyS->getlast(energy_current).toDouble();
  double pump2energyLast = pump2energyS->getlast(energy_current).toDouble();
  //  if (sensorses < 2 or coTherm < 0 or waterTherm < 0) {
  //    display.print("Err");
  //    digitalWrite(relay2, 0); digitalWrite(relay1, 0);
  //    delay(20000);
  //      resetFunc();
  //  }

#ifdef debug1
    Serial.println(String(millis())+F(": CheckPumps: coTherm+histereza > Watertemp: ")+String(coTherm + histereza  > waterTherm)+F(", coTherm: ")+String(coTherm)+F(", watertherm+histereza: ")+String( waterTherm + histereza));
//      if (((millis()-uptime) % 10000) == 0)  digitalWrite(relay1,!digitalRead(relay1)) ;
#endif
#ifdef enableWebSerial
    WebSerial.println(String(millis())+F(": CheckPumps: coTherm+histereza > Watertemp: ")+String(coTherm + histereza  > waterTherm )+F(", coTherm: ")+String(coTherm)+F(", watertherm+histereza: ")+String(waterTherm + histereza));
//      if (((millis()-uptime) % 10000) == 0)  digitalWrite(relay1,!digitalRead(relay1)) ;
#endif
  if (isnan(coTherm) or isnan(waterTherm) or coTherm >= panic or coTherm < 1 ) {
    //gdy zawioda czujniki lub panika -wlacz pompy
    //LOW wlacza  HIGH wylacza
    //LOW wlacza  HIGH wylacza
    //relay1 woda
    //relay2 CO
    #ifdef debug1
      Serial.print(String(millis())+F(": Force to panic... Change state relays to ON..."));
    #endif
    #ifdef enableWebSerial
      WebSerial.print(String(millis())+F(": Force to panic... Change state relays to ON..."));
    #endif
    prgstatusrelay1WO = HIGH;
    supla_and_relay_obsluga(pumpWaterRelay, HIGH); //WLACZ ZMIENILEM LOGIKE
    prgstatusrelay2CO = HIGH;
    supla_and_relay_obsluga(pumpCoRelay, HIGH);  //WLACZ ZMIENILEM LOGIKE
    panicbuz = true;
  } else {
    #ifdef debug1
      Serial.println(String(millis())+F(": No Panic loop......"));
    #endif
    #ifdef enableWebSerial
      WebSerial.println(String(millis())+F(": No Panic loop......"));
    #endif
    //zalozenie mamy prawidlowe odczyty i kontynuujemy kontrole
    panicbuz = false;
    if (pump1energyLast > pumpmincurrent and digitalRead(relay1) == HIGH) { // or (OutsideTempAvg>forceCObelow*1.05)) and coTherm > coConstTempCutOff)  { //digitalRead(relay1) == HIGH)  or (OutsideTempAvg>forceCObelow*1.05)) and coTherm > coConstTempCutOff)   {
        #ifdef debug1
        Serial.println(String(millis())+F(": wymys: dAJ H2O -forceWater"));
        #endif
        #ifdef enableWebSerial
          WebSerial.println(String(millis())+F(": wymys: dAJ H2O -forceWater"));
        #endif
        display.blink();
        forceWater=true;
    }

    if (pump2energyLast > pumpmincurrent and digitalRead(relay2) == HIGH) { //or (OutsideTempAvg<forceCObelow)) and coTherm > coConstTempCutOff)  {
        #ifdef debug1
          Serial.println(String(millis())+F(": wymys: dAJ dCO -forceCO"));
        #endif
        #ifdef enableWebSerial
        WebSerial.println(String(millis())+F(": wymys: dAJ dCO -forceCO"));
        #endif
        display.blink();
      forceCO=true;
    }
    if (coTherm < (coConstTempCutOff - histereza) and panicbuz == false ) {
      //wylacz bo nic sie nie dzieje i temp pieca<30-histereza stopni
      prgstatusrelay1WO = LOW; //supla_and_relay_obsluga(pumpWaterRelay, LOW); //WyLACZ ZMIENILEM LOGIKE
      prgstatusrelay2CO = LOW; //supla_and_relay_obsluga(pumpCoRelay, LOW); //WyLACZ ZMIENILEM LOGIKE
      forceCO=false;
      forceWater=false;
      if ( (waitCOStartingmargin + 6*60*60*1000) < millis()) najpierwCO=false; //opoznij 1 godzine wylaczenie najpierwCO przed wylaczeniem
      #ifdef debug1
        Serial.println(String(millis())+F(": Wylaczam pompy -za niska temperatura na piecu"));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(String(millis())+F(": Wylaczam pompy -za niska temperatura na piecu"));
      #endif
    } else {
      waitCOStartingmargin=millis();  //odswiezenie licznika bo inaczej moze przelaczyc po zdefiniowanym czasie grzania
      #ifdef debug1
        Serial.println(String(millis())+F(": Grzejemy się ;) -mamy temp w piecu"));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(String(millis())+F(": Grzejemy się ;) -mamy temp w piecu"));
      #endif
      if ((coTherm + histereza) > waterTherm or forceWater == true) { //or (forceWater==true  and forceCO==false)) {
        #ifdef debug1
          Serial.println(String(millis())+F(": gdy temp pieca wieksza od wody -wlacz by zagrzac"));
        #endif //gdy temp pieca wieksza od wody -wlacz by zagrzac
        #ifdef enableWebSerial
        WebSerial.println(String(millis())+F(": gdy temp pieca wieksza od wody -wlacz by zagrzac"));
        #endif
        prgstatusrelay1WO = HIGH; //supla_and_relay_obsluga(pumpWaterRelay, HIGH);    //wlacz wode i wylacz co  //WLACZ ZMIENILEM LOGIKE
        prgstatusrelay2CO = LOW; //supla_and_relay_obsluga(pumpCoRelay, LOW);  //WYLACZ ZMIENILEM LOGIKE
    //    forceCO=false;
      } else
      if ((( coTherm + histereza) < waterTherm  ) or najpierwCO == true) { //or forceCO == true) {  //and OutsideTempAvg < forceCObelow*1.05
        #ifdef debug1
          Serial.println(F("wlacz CO podlogowek gdy grzanie wody wylaczone "));
        #endif
        #ifdef enableWebSerial
        WebSerial.println(F("wlacz CO podlogowek gdy grzanie wody wylaczone "));
        #endif
        //wlacz CO podlogowek gdy grzanie wody wylaczone i srednia na zewnatrz temp <24stopni
        //oraz gdy temp CO<temp wody w baniaku
        prgstatusrelay1WO = LOW; //supla_and_relay_obsluga(pumpWaterRelay, LOW);  //WYLACZ ZMIENILEM LOGIKE
        if (OutsideTempAvg < forceCObelow) prgstatusrelay2CO = HIGH;   //ogranicz grzanie co gdy temp na zewnątrz <10
      } else
      // if (OutsideTempAvg > forceCObelow*1.05 ) { //and forceCO == true) {
      //   #ifdef debug1
      //     Serial.println(String(millis())+F(": Disable CO "));
      //   #endif
      //   #ifdef enableWebSerial
      //   WebSerial.println(String(millis())+F(": Disable CO "));
      //   #endif
      //   prgstatusrelay2CO = LOW;
      // } else {
      //   forceCO = true;
      //   displayCoCo();
      // }
      if (forceWater == true and coTherm < (coConstTempCutOff - histereza) and panicbuz == false) {
        #ifdef debug1
          Serial.println(String(millis())+F(": ForceWater "));
        #endif
        #ifdef enableWebSerial
        WebSerial.println(String(millis())+F(": ForceWater "));
        #endif
        prgstatusrelay1WO = HIGH; //supla_and_relay_obsluga(pumpWaterRelay, HIGH);    //wlacz wode i co  //WLACZ ZMIENILEM LOGIKE
        if (forceCO == true) {prgstatusrelay2CO = HIGH;} else {prgstatusrelay2CO = LOW;}
      } else
      if (forceCO == true and coTherm < (coConstTempCutOff - histereza) and panicbuz == false) {
        #ifdef debug1
          Serial.println(String(millis())+F(": Force CO"));
        #endif
        #ifdef enableWebSerial
        WebSerial.println(String(millis())+F(": Force CO"));
        #endif
        prgstatusrelay2CO = HIGH; //supla_and_relay_obsluga(pumpCoRelay, HIGH);   //WLACZ ZMIENILEM LOGIKE
        if (forceWater == true) {prgstatusrelay1WO = HIGH;} else {prgstatusrelay1WO = LOW;}
      }
    }
  }
  supla_and_relay_obsluga(pumpWaterRelay, prgstatusrelay1WO);    //wlacz wode i wylacz co  //WLACZ ZMIENILEM LOGIKE
  supla_and_relay_obsluga(pumpCoRelay, prgstatusrelay2CO);  //WYLACZ ZMIENILEM LOGIKE
}
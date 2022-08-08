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

//*************************************************0x28, 0xFF, 0x4B, 0x72, 0x80, 0x14, 0x02, 0x77
#include "main.h"

//*************************************************


void setup()
{
//#if defined(debug) or defined(debug1)
//   Serial.begin(74880);
//   Serial.print("\nStarting Async_AutoConnect_ESP8266_minimal on " + String(ARDUINO_BOARD) + String("  "));
//  // #endif
//   // first get total memory before we do anything
//   getFreeMemory();
//   #ifdef doubleResDet
//   // double reset detect from start
//   doubleResetDetect();
//   #endif

  MainCommonSetup();

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
  log_message(log_chars,0);
  Ro = MQCalibration(gas_ain);
  getCOGAS(gas_ain);
#ifdef debug
  log_message((char*)F("gas ok..."));
#endif

#ifdef debug
  log_message((char*)F("end setup...."));
  //    SaveConfig();
#endif

}

void getCOGAS(int pinpriv)
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
  if (isadslinitialised)
  {
    if ((((millis() - lastEnergyRead) > ReadEnergyTimeInterval) or lastEnergyRead == 0))
    {
      if (isnan(energy1used)) energy1used = 0;
      if (isnan(energy2used)) energy2used = 0;
      lastEnergyRead = millis();
      pump1energyLast = getenergy(ads_0);                  //to check if pump is running
      if (isnan(pump1energyLast)) pump1energyLast = 0;
      energy1used += getkWh(pump1energyLast);
      pump2energyLast = getenergy(ads_1);                   //to check if pump is running
      if (isnan(pump2energyLast)) pump2energyLast = 0;
      energy2used += getkWh(pump2energyLast);
      if (pump1energyLast > pumpmincurrent or pump2energyLast > pumpmincurrent)     //limit log to usable datas
      {
        sprintf(log_chars,"energy measured 1: %s, 2: %s  energy used: 1: %s  2: %s",String(pump1energyLast,4).c_str(),String(pump2energyLast,4).c_str(),String(energy1used,4).c_str(),String(energy2used,4).c_str());
        log_message(log_chars,0);
      }
    }
  }
  allEnergy = energy1used + energy2used;
}

void ReadTemperatures()
{
  log_message((char*)F("Read Temperatures starting"));
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
    log_message(log_chars,0);
    if (count == 0)
      count = maxsensors;
    String temptmp = " Collecting 18B20 ROMS and temps:\n";
    for (int j = 0; j < count; j++)
    {
      temp1w = sensors.getTempCByIndex(j);
      if (!check_isValidTemp(temp1w))
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

      String addstrtmp = String(coThermometerAddr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (check_isValidTemp(temp1w)) coTherm = temp1w; assignedsensor = true; }
      addstrtmp = String(waterThermometerAddr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (check_isValidTemp(temp1w)) waterTherm = temp1w; assignedsensor = true; }
      addstrtmp = String(NThermAddr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (check_isValidTemp(temp1w)) NTherm = temp1w; assignedsensor = true; }
      addstrtmp = String(WThermAddr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (check_isValidTemp(temp1w)) WTherm = temp1w; assignedsensor = true; }
      addstrtmp = String(EThermAddr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (check_isValidTemp(temp1w)) ETherm = temp1w; assignedsensor = true; }
      addstrtmp = String(SThermAddr); addstrtmp.toUpperCase(); addstrtmp.trim();
      if (addrstr == addstrtmp) { if (check_isValidTemp(temp1w)) STherm = temp1w; assignedsensor = true; }

      if (!assignedsensor and UnassignedTempSensor.indexOf(addrstr) == -1 and check_isValidTemp(temp1w)) UnassignedTempSensor += String(addrstr) + " : " + String(temp1w) + "\n";  //
      assignedsensor = false;
      if (check_isValidTemp(temp1w)) temptmp += "       " + String(j) + ": 18B20 ROM= " + addrstr + ", temp: " + String(temp1w, 2) + "\n";
    }
    OutsideTempAvg = ((!check_isValidTemp(NTherm) ? 0 : NTherm) + (!check_isValidTemp(ETherm) ? 0 : ETherm) + (!check_isValidTemp(WTherm) ? 0 : WTherm) + (!check_isValidTemp(STherm) ? 0 : STherm)) / ((!check_isValidTemp(NTherm) ? 0 : 1) + (!check_isValidTemp(ETherm) ? 0 : 1) + (!check_isValidTemp(WTherm) ? 0 : 1) + (!check_isValidTemp(STherm) ? 0 : 1));
    if (!check_isValidTemp(OutsideTempAvg)) OutsideTempAvg = InitTemp;
    log_message((char *)temptmp.c_str());
    UnassignedTempSensor.trim();
    if (UnassignedTempSensor != "")
    {
      sprintf(log_chars, "Unassigned Sensors: %s and AvgOutside Temp: %s", UnassignedTempSensor.c_str(), String(OutsideTempAvg,2).c_str());
      log_message(log_chars,0);
    }

  if (ExistBM280)
  {
    bmTemp = bmp.readTemperature();
    if (!check_isValidTemp(bmTemp)) bmTemp = InitTemp;
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
      // if (!check_isValidTemp(t)) t=InitTemp;
      // if (isnan(h)) h=0;
    }
    else
    {
      dhtreadtime = millis();
    }

    sprintf(log_chars, "Get DHT values t= %s, humid: %s", String(t, 2), String(h, 1));
    log_message(log_chars,0);
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

uint counter = 0;

void checkAndRunCommandBySwitch()
{
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
}

void loop()
{
  MainCommonLoop();
  counter++;

  ReadTemperatures();
  ReadEnergyUsed();

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

  checkAndRunCommandBySwitch();


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


  if ((millis() - savetime > (saveminut)) and (allEnergy != (energy1used + energy2used)))
  {
    SaveEnergy();
    sprintf(log_chars, "Saved energy after savetime, allEn: %s en1: %s en2: %s", String(allEnergy).c_str(), String(energy1used).c_str(), String(energy2used).c_str());
    log_message(log_chars,0);
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
    getCOGAS(gas_ain);
    log_message((char *)F("Check gas leak"));
    gas_leak_check();
    // check_temps_pumps();
  }

  if (counter % (10 * last_case + what_display) * 2.3 == 0 or millis() < 1000)
  { // 4casy
    display_temp_rotation();
  }

  if (counter % 100 == 0)
  {
    log_message((char *)F("check_temps_pumps();"));
    check_temps_pumps();
  }

}

void ChangeRelayStatus(uint8_t numer, bool stan)
{
  // LOW wlacza  HIGH wylacza
  sprintf_P(log_chars, "ChangeRelayStatus. pompa: %s, Stan: %s, prgstatusrelay1WO: %s, prgstatusrelay2CO: %s", (numer == pumpWaterRelay ? "WODA" : "CO"), stan ? "ON" : "OFF", prgstatusrelay1WO ? "ON" : "OFF", prgstatusrelay2CO ? "ON" : "OFF");
  log_message(log_chars,0);
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
  log_message(log_chars,0);
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
  else {
    log_message((char*)F("No Panic loop......"));
    //zalozenie mamy prawidlowe odczyty i kontynuujemy kontrole
    panicbuz = false;
    if (pump1energyLast > pumpmincurrent and digitalRead(relay1) == HIGH) { // or (OutsideTempAvg>forceCObelow*1.05)) and coTherm > coConstTempCutOff)  { //digitalRead(relay1) == HIGH)  or (OutsideTempAvg>forceCObelow*1.05)) and coTherm > coConstTempCutOff)   {
        log_message((char*)F("WYMUŚ: DAJ H2O -forceWater"));
        display.blink();
        forceWater=true;
    }
    if (pump2energyLast > pumpmincurrent and digitalRead(relay2) == HIGH) { //or (OutsideTempAvg<forceCObelow)) and coTherm > coConstTempCutOff)  {
        log_message((char*)F("WYMUŚ: DAJ dCO -forceCO"));
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
      log_message((char*)F("Wylaczam pompy -za niska temperatura na piecu"));
    } else {
      waitCOStartingmargin=millis();  //odswiezenie licznika bo inaczej moze przelaczyc po zdefiniowanym czasie grzania
      log_message((char*)F("Grzejemy się ;) -mamy temp na piecu"));
      if ((coTherm + histereza) > waterTherm or forceWater == true) { //or (forceWater==true  and forceCO==false)) {
        log_message((char*)F("gdy temp pieca wieksza od wody -wlacz by zagrzac"));
        prgstatusrelay1WO = HIGH; //supla_and_relay_obsluga(pumpWaterRelay, HIGH);    //wlacz wode i wylacz co  //WLACZ ZMIENILEM LOGIKE
        prgstatusrelay2CO = LOW; //supla_and_relay_obsluga(pumpCoRelay, LOW);  //WYLACZ ZMIENILEM LOGIKE
    //    forceCO=false;
      } else
      if ((( coTherm + histereza) < waterTherm  ) or najpierwCO == true) { //or forceCO == true) {  //and OutsideTempAvg < forceCObelow*1.05
        log_message((char*)F("wlacz CO podlogowek gdy grzanie wody wylaczone"));
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
        log_message((char*)F("Force Water"));
        prgstatusrelay1WO = HIGH; //supla_and_relay_obsluga(pumpWaterRelay, HIGH);    //wlacz wode i co  //WLACZ ZMIENILEM LOGIKE
        if (forceCO == true) {prgstatusrelay2CO = HIGH;} else {prgstatusrelay2CO = LOW;}
      } else
      if (forceCO == true and coTherm < (coConstTempCutOff - histereza) and panicbuz == false) {
        log_message((char*)F("Force CO"));
        prgstatusrelay2CO = HIGH; //supla_and_relay_obsluga(pumpCoRelay, HIGH);   //WLACZ ZMIENILEM LOGIKE
        if (forceWater == true) {prgstatusrelay1WO = HIGH;} else {prgstatusrelay1WO = LOW;}
      }
    }
  }
  ChangeRelayStatus(pumpWaterRelay, prgstatusrelay1WO);
  ChangeRelayStatus(pumpCoRelay, prgstatusrelay2CO);
}

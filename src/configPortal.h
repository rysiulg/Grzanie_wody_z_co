
#ifdef enableWifiManager
#if !( defined(ESP8266) )
  #error This code is intended to run on ESP8266 platform! Please check your Tools->Board setting.
#endif

#define USE_AVAILABLE_PAGES     true
#ifdef USE_ESP_WIFIMANAGER_NTP
#undef USE_ESP_WIFIMANAGER_NTP
#endif
#define USE_ESP_WIFIMANAGER_NTP    false
#ifdef USE_CLOUDFLARE_NTP
#undef USE_CLOUDFLARE_NTP
#endif
#define USE_CLOUDFLARE_NTP          false

// New in v1.0.11
#ifdef USING_CORS_FEATURE
#undef USING_CORS_FEATURE
#endif
#define USING_CORS_FEATURE          true

#define USE_DHCP_IP     true


//******************************************************************************************
void ondemandwifiCallback() {

  display.print("COnF");
  webserver.end();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  String ssid = "ESP_" + String(ESP_getChipId(), HEX);
//  ESPAsync_WiFiManager ESPAsync_wifiManager(&server, &dns);//, "ESP_" + String(ESP_getChipId(), HEX));
  wdt_reset();
  wdt_disable();
  hw_wdt_disable();
 // wdt_enable(WDTO_8S);
  String nowe18baddr = checkUnassignedSensors() ;
  char nowe18baddrchar[nowe18baddr.length()];

#ifdef debug
  Serial.println("Conversja to chararrays");
#endif
  nowe18baddr.toCharArray(nowe18baddrchar, nowe18baddr.length());
#ifdef debug
  Serial.print("z char: ");  Serial.println(nowe18baddrchar);
#endif
#ifdef WIFI_MANAGER_MAX_PARAMS
#undef WIFI_MANAGER_MAX_PARAMS
#endif
#define WIFI_MANAGER_MAX_PARAMS 15

  // Use only to erase stored WiFi Credentials
  //resetSettings();
  //ESPAsync_wifiManager.resetSettings();

  ESPAsync_WMParameter   custom_MQTT_server("MQTT_server", "ha", mqtt_server, 81, "required");
  ESPAsync_WMParameter   custom_MQTT_username("MQTT_username", "mqqt", mqtt_user, 51, "required");
  ESPAsync_WMParameter   custom_MQTT_password("MQTT_password", "MQTT password", mqtt_password, 51, "required");
  char tmp[17], ntmp[32];
  array_to_string(sts[0].ssn, 8, tmp);
  String(sts[0].sname).toCharArray(ntmp, String(sts[0].sname).length());
  Serial.println(ntmp);
  ESPAsync_WMParameter   custom_18B20_name("sns0", sts[0].sname, tmp, 18, "required");
  array_to_string(sts[1].ssn, 8, tmp);
  String(sts[1].sname).toCharArray(ntmp, String(sts[1].sname).length());
  ESPAsync_WMParameter   custom_18B20_name1("sns1", sts[1].sname, tmp, 18, "required");
  array_to_string(sts[2].ssn, 8, tmp);
  String(sts[2].sname).toCharArray(ntmp, String(sts[2].sname).length());
  ESPAsync_WMParameter   custom_18B20_name2("sns2", sts[2].sname, tmp, 18, "required");
  array_to_string(sts[3].ssn, 8, tmp);
  String(sts[3].sname).toCharArray(ntmp, String(sts[3].sname).length());
  ESPAsync_WMParameter   custom_18B20_name3("sns3", sts[3].sname, tmp, 18, "required");
  array_to_string(sts[4].ssn, 8, tmp);
  String(sts[4].sname).toCharArray(ntmp, String(sts[4].sname).length());
  ESPAsync_WMParameter   custom_18B20_name4("sns4", sts[4].sname, tmp, 18, "required");
  array_to_string(sts[5].ssn, 8, tmp);
  String(sts[5].sname).toCharArray(ntmp, String(sts[5].sname).length());
  ESPAsync_WMParameter   custom_18B20_name5("sns5", sts[5].sname, tmp, 18, "required");
  array_to_string(sts[6].ssn, 8, tmp);
  String(sts[6].sname).toCharArray(ntmp, String(sts[6].sname).length());
  ESPAsync_WMParameter   custom_18B20_name6("sns6", sts[6].sname, tmp, 18, "required");
  ESPAsync_WMParameter   custom_text(nowe18baddrchar);
  ESPAsync_WMParameter   custom_resetEnergy("RESET Energy meter","RESET Energy","", 5);
  //String("Supla Last State").toCharArray(ntmp, String("Supla Last State").length());
//  ESPAsync_WMParameter   custom_Supla_status("status", "Supla Last State", Supla_status, 51, "readonly");
  //
  //// replace WiFiManager.h by this line to see names before values: const char HTTP_FORM_PARAM[] PROGMEM      = "<br/>{p}<input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
  //
wdt_reset();
    ESPAsync_WiFiManager  wifiManager(&webserver, &dns, ssid.c_str());

  #ifdef debug
    Serial.println(F(" -asynNot conn"));
  #endif
  //wifiManager.setDebugOutput(true);
  ////  ESP_WiFiManager wifiManager;
  //  wifiManager.setBreakAfterConfig(true);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_MQTT_server);
  wifiManager.addParameter(&custom_MQTT_username);
  wifiManager.addParameter(&custom_MQTT_password);
  wifiManager.addParameter(&custom_text);
  wifiManager.addParameter(&custom_18B20_name);
  wifiManager.addParameter(&custom_18B20_name1);
  wifiManager.addParameter(&custom_18B20_name2);
  wifiManager.addParameter(&custom_18B20_name3);
  wifiManager.addParameter(&custom_18B20_name4);
  wifiManager.addParameter(&custom_18B20_name5);
  wifiManager.addParameter(&custom_18B20_name6);
  wifiManager.addParameter(&custom_resetEnergy);


//  wifiManager.addParameter(&custom_Supla_status);
//  wifiManager.setCustomHeadElement("<style>html{ background-color: #01DF3A;}</style><div class='s'><svg version='1.1' width='75px' height='75px' id='l' x='0' y='0' viewBox='0 0 200 200' xml:space='preserve'><path d='M59.3,2.5c18.1,0.6,31.8,8,40.2,23.5c3.1,5.7,4.3,11.9,4.1,18.3c-0.1,3.6-0.7,7.1-1.9,10.6c-0.2,0.7-0.1,1.1,0.6,1.5c12.8,7.7,25.5,15.4,38.3,23c2.9,1.7,5.8,3.4,8.7,5.3c1,0.6,1.6,0.6,2.5-0.1c4.5-3.6,9.8-5.3,15.7-5.4c12.5-0.1,22.9,7.9,25.2,19c1.9,9.2-2.9,19.2-11.8,23.9c-8.4,4.5-16.9,4.5-25.5,0.2c-0.7-0.3-1-0.2-1.5,0.3c-4.8,4.9-9.7,9.8-14.5,14.6c-5.3,5.3-10.6,10.7-15.9,16c-1.8,1.8-3.6,3.7-5.4,5.4c-0.7,0.6-0.6,1,0,1.6c3.6,3.4,5.8,7.5,6.2,12.2c0.7,7.7-2.2,14-8.8,18.5c-12.3,8.6-30.3,3.5-35-10.4c-2.8-8.4,0.6-17.7,8.6-22.8c0.9-0.6,1.1-1,0.8-2c-2-6.2-4.4-12.4-6.6-18.6c-6.3-17.6-12.7-35.1-19-52.7c-0.2-0.7-0.5-1-1.4-0.9c-12.5,0.7-23.6-2.6-33-10.4c-8-6.6-12.9-15-14.2-25c-1.5-11.5,1.7-21.9,9.6-30.7C32.5,8.9,42.2,4.2,53.7,2.7c0.7-0.1,1.5-0.2,2.2-0.2C57,2.4,58.2,2.5,59.3,2.5z M76.5,81c0,0.1,0.1,0.3,0.1,0.6c1.6,6.3,3.2,12.6,4.7,18.9c4.5,17.7,8.9,35.5,13.3,53.2c0.2,0.9,0.6,1.1,1.6,0.9c5.4-1.2,10.7-0.8,15.7,1.6c0.8,0.4,1.2,0.3,1.7-0.4c11.2-12.9,22.5-25.7,33.4-38.7c0.5-0.6,0.4-1,0-1.6c-5.6-7.9-6.1-16.1-1.3-24.5c0.5-0.8,0.3-1.1-0.5-1.6c-9.1-4.7-18.1-9.3-27.2-14c-6.8-3.5-13.5-7-20.3-10.5c-0.7-0.4-1.1-0.3-1.6,0.4c-1.3,1.8-2.7,3.5-4.3,5.1c-4.2,4.2-9.1,7.4-14.7,9.7C76.9,80.3,76.4,80.3,76.5,81z M89,42.6c0.1-2.5-0.4-5.4-1.5-8.1C83,23.1,74.2,16.9,61.7,15.8c-10-0.9-18.6,2.4-25.3,9.7c-8.4,9-9.3,22.4-2.2,32.4c6.8,9.6,19.1,14.2,31.4,11.9C79.2,67.1,89,55.9,89,42.6z M102.1,188.6c0.6,0.1,1.5-0.1,2.4-0.2c9.5-1.4,15.3-10.9,11.6-19.2c-2.6-5.9-9.4-9.6-16.8-8.6c-8.3,1.2-14.1,8.9-12.4,16.6C88.2,183.9,94.4,188.6,102.1,188.6z M167.7,88.5c-1,0-2.1,0.1-3.1,0.3c-9,1.7-14.2,10.6-10.8,18.6c2.9,6.8,11.4,10.3,19,7.8c7.1-2.3,11.1-9.1,9.6-15.9C180.9,93,174.8,88.5,167.7,88.5z'/></svg>"); //
  //  wifiManager.setMinimumSignalQuality(8);
//     wifiManager.setShowStaticFields(false); // force show static ip fields
//     wifiManager.setShowDnsFields(false);    // force show dns field always

  wifiManager.setConfigPortalTimeout(900);
  wifiManager.setMinimumSignalQuality(-1);
//  wifiManager.autoConnect(ssid);
  wifiManager.setConfigPortalChannel(0);


  //  wifiManager.resetSettings();
  wifiManager.autoConnect(ssid.c_str());
wifiManager.startConfigPortal();
//  if (!wifiManager.startConfigPortal(ssid.c_str())) {
//    Serial.println("Not connected to WiFi but continuing anyway.");
//  } else {
//    Serial.println("connected...yeey :)");
//  }
  wdt_enable(WDTO_8S);
  hw_wdt_enable();
wdt_reset();
  char *resetword=(char*)"RESE0";
  strcpy(mqtt_server, custom_MQTT_server.getValue());
  strcpy(mqtt_user, custom_MQTT_username.getValue());
  strcpy(mqtt_password, custom_MQTT_password.getValue());
  //if (mqtt_password == NULL) memset(MQTT_password,0,sizeof(MQTT_password));
  strcpy(resetword, custom_resetEnergy.getValue());
  #ifdef debug
    Serial.print(F("Reset: "));
    Serial.println(resetword);
  #endif
  if (String(resetword)=="RESET") {
    energy1used=0; energy2used=0;
    SaveEnergy();
    Serial.println("On reset seve config");
  }
  //tmp='';
  strcpy(tmp, custom_18B20_name.getValue());
  #ifdef debug
    Serial.print(F("18B20: "));
    Serial.println(tmp);
  #endif
  hexCharacterStringToBytes(sts[0].ssn, tmp);
  strcpy(tmp, custom_18B20_name1.getValue());
  #ifdef debug
    Serial.print(F("18B20: "));
    Serial.println(tmp);
  #endif
  hexCharacterStringToBytes(sts[1].ssn, tmp);
  strcpy(tmp, custom_18B20_name2.getValue());
  #ifdef debug
    Serial.print(F("18B20: "));
    Serial.println(tmp);
  #endif
  hexCharacterStringToBytes(sts[2].ssn, tmp);
  strcpy(tmp, custom_18B20_name3.getValue());
  #ifdef debug
    Serial.print(F("18B20: "));
    Serial.println(tmp);
  #endif
  hexCharacterStringToBytes(sts[3].ssn, tmp);
  strcpy(tmp, custom_18B20_name4.getValue());
  #ifdef debug
    Serial.print(F("18B20: "));
    Serial.println(tmp);
  #endif
  hexCharacterStringToBytes(sts[4].ssn, tmp);
  strcpy(tmp, custom_18B20_name5.getValue());
  #ifdef debug
    Serial.print(F("18B20: "));
    Serial.println(tmp);
  #endif
  hexCharacterStringToBytes(sts[5].ssn, tmp);
  strcpy(tmp, custom_18B20_name6.getValue());
  #ifdef debug
    Serial.print(F("18B20: "));
    Serial.println(tmp);
  #endif
  hexCharacterStringToBytes(sts[6].ssn, tmp);

//  if (strcmp(Supla_server, "get_new_guid_and_authkey") == 0) {
#ifdef debug
    Serial.println("new guid & authkey. In wifimanager");
#endif
//    EEPROM.write(300, 0);
//    EEPROM.commit();
//    WiFi.forceSleepBegin();
    saveConfigCallback();
    wdt_reset(); ESP.restart(); while (1)ESP.restart(); wdt_reset(); ESP.restart();

//  WiFi.softAPdisconnect(true);   //  close AP
}
#endif
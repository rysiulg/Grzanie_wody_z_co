

void mqttReconnect_subscribe_list()
{
#if defined enableMQTT || defined enableMQTTAsync
    log_message((char*)F("MQTT Reconnect Subscribe List..."));
    SubscribeMQTT(SUPLA_VOLT_TOPIC, QOS);
    SubscribeMQTT(SUPLA_FREQ_TOPIC, QOS);
    SubscribeMQTT(BOILER_GAZ_CWU_TOPIC, QOS);
    String tmp = "\0";
    tmp = BOILERROOM_SWITCH_TOPIC_SET;
    tmp += F("_");
    tmp += BOILERROOM_PUMP1WA;
    SubscribeMQTT(tmp, QOS);
    tmp = BOILERROOM_SWITCH_TOPIC_SET;
    tmp += F("_");
    tmp += BOILERROOM_PUMP2CO;
    SubscribeMQTT(tmp, QOS);
#endif
}

#if defined enableMQTT || defined enableMQTTAsync
void mqttCallbackAsString(String &topicStrFromMQTT, String &payloadStrFromMQTT) {
//get Voltage on same phase
  if (topicStrFromMQTT.indexOf(String(BOILER_GAZ_CWU_TOPIC))==0 and payloadStrFromMQTT.indexOf(String(BOILER_GAZ_CWU_JSON))>=0)              //NEWS averange temp -outside temp
  {
    String ident = "BOILER_GAZ_CWU_JSON temp ";
    String tmpStrmqtt = getJsonVal(payloadStrFromMQTT, String(BOILER_GAZ_CWU_JSON));
    if (PayloadtoValidFloatCheck(tmpStrmqtt))          //invalid val is displayed in funct
    {
      waterThermBG = PayloadtoValidFloat(tmpStrmqtt, true);   //true to get output to serial and webserial
      sprintf(log_chars, "%s updated from MQTT to: %s", ident.c_str(), String(waterThermBG).c_str());
      log_message(log_chars);
      //      receivedmqttdata = true;    //makes every second run mqtt send and influx
    } else {
      //sprintf(log_chars, "%s not updated from MQTT: %s, %s", ident.c_str(), getJsonVal(payloadStrFromMQTT, String(NEWStemp_json)).c_str(), String(PayloadtoValidFloatCheck(getJsonVal(payloadStrFromMQTT, String(NEWStemp_json)))).c_str());
      log_message(log_chars);
    }
  } else
  if (topicStrFromMQTT == SUPLA_VOLT_TOPIC)
  {
    String ident = String(millis())+F(": electricmain Volt ");
    if (PayloadtoValidFloatCheck(payloadStrFromMQTT))
    {
      commonVolt = PayloadtoValidFloat(payloadStrFromMQTT,true);     //true to get output to serial and webserial atof(paylstr.c_str());
      // char result[6];
      // dtostrf(commonVolt, 4, 2, result);
//      receivedmqttdata = true;    //makes every second run mqtt send and influx
    }
  } else
//GET FREQ from supla
  if (topicStrFromMQTT == SUPLA_FREQ_TOPIC)
  {
    String ident = String(millis())+F(": electricmain Frequency ");
    if (PayloadtoValidFloatCheck(payloadStrFromMQTT))
    {
      commonFreq = PayloadtoValidFloat(payloadStrFromMQTT,true);     //true to get output to serial and webserial atof(paylstr.c_str());
        // commonFreq = atof(paylstr.c_str());
        // char result[6];
        // dtostrf(commonFreq, 4, 2, result);
    }
  } else
//PUMP1
  if (topicStrFromMQTT == BOILERROOM_SWITCH_TOPIC_SET+"_"+BOILERROOM_PUMP1WA)
  {
    String ident = String(F("Boiler PUMP1 Wather. "));
    payloadStrFromMQTT.toUpperCase();
    ident += F("Set mode: ");
    bool tmp = PayloadStatus(payloadStrFromMQTT, true) && !PayloadStatus(payloadStrFromMQTT, false);
    if (tmp != prgstatusrelay1WO) receivedmqttdata = true;
    if (PayloadStatus(payloadStrFromMQTT, true))
    {
      #ifndef enableWebSocket
      ownother=F("<B> MQTT pump1WO ")+payloadStrFromMQTT+F("</B>");
      #endif
      prgstatusrelay1WO = true;
      forceWater = true;
      forceCO = false;
      najpierwCO = false;
      ident += "WO mode " + payloadStrFromMQTT;
    }
    else if (PayloadStatus(payloadStrFromMQTT, false))
    {
      prgstatusrelay1WO = false;
      forceWater = false;
      forceCO = false;
      najpierwCO = false;
      ident +="WO mode " + payloadStrFromMQTT;
    }
    else {
      ident += F(" unknkown ");
    }
    log_message((char*)ident.c_str());
  } else
//PUMP2
  if (topicStrFromMQTT == BOILERROOM_SWITCH_TOPIC_SET+"_"+BOILERROOM_PUMP2CO)
  {
    String ident = F("Boiler PUMP2 CO. ");
    payloadStrFromMQTT.toUpperCase();
    ident += F("Set mode: ");
    bool tmp = PayloadStatus(payloadStrFromMQTT, true) && !PayloadStatus(payloadStrFromMQTT, false);
    if (tmp != prgstatusrelay2CO) receivedmqttdata = true;
    if (PayloadStatus(payloadStrFromMQTT, true))
    {
      #ifndef enableWebSocket
      ownother=F("<B> MQTT pump2CO ")+payloadStrFromMQTT+F("</B>");
      #endif
      prgstatusrelay2CO = true;
      forceCO = true;
      waitCOStartingmargin=millis();
      forceWater = false;
      najpierwCO = true;
      ident +="CO mode " + payloadStrFromMQTT;
    }
    else if (PayloadStatus(payloadStrFromMQTT, false))
    {
      prgstatusrelay2CO = false;
      forceCO = false;
      forceWater = false;
      najpierwCO = false;
      ident += "CO mode " + payloadStrFromMQTT;
    }
    else {
      ident += F(" unknkown ");
    }
    log_message((char*)ident.c_str());
  }
   receivedmqttdata = false;
}
#endif

#if defined enableMQTT || defined enableMQTTAsync

void updateMQTTData()
{
  String mqttdeviceid = String(BASE_TOPIC);
  const String payloadvalue_startend_val = F(" "); // value added before and after value send to mqtt queue
  String tmpbuilder = F("\0");
  // char mqttTopic[maxLenMQTTTopic] = {'\0'};
  // char mqttPayload[maxLenMQTTTopic] = {'\0'};
  // sprintf(mqttTopic, BR_OUTSIDE_SENSOR_TOPIC.c_str());
  String mqttTopic = String(BR_OUTSIDE_SENSOR_TOPIC);
  if (check_isValidTemp(NTherm)) { tmpbuilder += build_JSON_Payload(OUTSIDE_TEMPERATURE_N, String(NTherm), true, payloadvalue_startend_val); }
  if (check_isValidTemp(ETherm)) { tmpbuilder += build_JSON_Payload(OUTSIDE_TEMPERATURE_E, String(ETherm), false, payloadvalue_startend_val); }
  if (check_isValidTemp(WTherm)) { tmpbuilder += build_JSON_Payload(OUTSIDE_TEMPERATURE_W, String(WTherm), false, payloadvalue_startend_val); }
  if (check_isValidTemp(STherm)) { tmpbuilder += build_JSON_Payload(OUTSIDE_TEMPERATURE_S, String(STherm), false, payloadvalue_startend_val); }
  if (check_isValidTemp(OutsideTempAvg)) { tmpbuilder += build_JSON_Payload(OUTSIDE_TEMPERATURE_A, String(OutsideTempAvg), false, payloadvalue_startend_val); }
  //sprintf(mqttPayload, tmpbuilder.c_str());
  publishMQTT(mqttTopic, tmpbuilder, mqtt_Retain, QOS);

  // sprintf(mqttTopic, BOILERROOM_SENSOR_TOPIC.c_str());
  mqttTopic = String(BOILERROOM_SENSOR_TOPIC);
  tmpbuilder = F("\0");
  tmpbuilder += build_JSON_Payload(BOILERROOM_PRESSURE, String(dbmpressval), true, payloadvalue_startend_val);
  if (check_isValidTemp(coTherm)) { tmpbuilder += build_JSON_Payload(HEATERCO_TEMPERATURE, String(coTherm), false, payloadvalue_startend_val); }
  if (check_isValidTemp(waterTherm)) { tmpbuilder += build_JSON_Payload(WATER_TEMPERATURE, String(waterTherm), false, payloadvalue_startend_val); }
  if (check_isValidTemp(bmTemp)) { tmpbuilder += build_JSON_Payload(BOILERROOM_TEMPERATURE, String(bmTemp), false, payloadvalue_startend_val); }
  tmpbuilder += build_JSON_Payload(BOILERROOM_HIGH, String(bm_high), false, payloadvalue_startend_val);
  tmpbuilder += build_JSON_Payload(BOILERROOM_HIGHREAL, String(bm_high_real), false, payloadvalue_startend_val);
  tmpbuilder += build_JSON_Payload(BOILERROOM_COVAL, String(dcoval), false, payloadvalue_startend_val);
  // sprintf(mqttPayload, tmpbuilder.c_str());
  publishMQTT(mqttTopic, tmpbuilder, mqtt_Retain, QOS);

  // sprintf(mqttTopic, BR_MEDIA_SENSOR_TOPIC.c_str());
  mqttTopic = String(BR_MEDIA_SENSOR_TOPIC);
  tmpbuilder = F("\0");
  tmpbuilder += build_JSON_Payload(BOILERROOM_PUMP1WA_E, String(energy1used), true, payloadvalue_startend_val);
  tmpbuilder += build_JSON_Payload(BOILERROOM_PUMP2CO_E, String(energy2used), false, payloadvalue_startend_val);
  // sprintf(mqttPayload, tmpbuilder.c_str());
  publishMQTT(mqttTopic, tmpbuilder, mqtt_Retain, QOS);

  // sprintf(mqttTopic, BOILERROOM_SWITCH_TOPIC.c_str());
  mqttTopic = String(BOILERROOM_SWITCH_TOPIC);
  tmpbuilder = F("\0");
  tmpbuilder += build_JSON_Payload(BOILERROOM_PUMP1WA, String(prgstatusrelay1WO?"\"ON\"":"\"OFF\""), true, payloadvalue_startend_val);
  tmpbuilder += build_JSON_Payload(BOILERROOM_PUMP2CO, String(prgstatusrelay1WO?"\"ON\"":"\"OFF\""), false, payloadvalue_startend_val);
  // sprintf(mqttPayload, tmpbuilder.c_str());
  publishMQTT(mqttTopic, tmpbuilder, mqtt_Retain, QOS);

  publishhomeassistantconfig++; // zwiekszamy licznik wykonan wyslania mqtt by co publishhomeassistantconfigdivider wysłań wysłać autoconfig discovery dla homeassisatnt
  if (publishhomeassistantconfig % publishhomeassistantconfigdivider == 0)
  {
    //Make Homeassistant autodiscovery and autoconfig
    //Temperatures
    if (!check_isValidTemp(NTherm)) HADiscovery(BR_OUTSIDE_SENSOR_TOPIC, "\0", OUTSIDE_TEMPERATURE_N, BOILERROOM_HA_SENSOR_TOPIC, "temperature");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_N, dNThermometerS, mqtt_HAClass_temperature);
    if (!check_isValidTemp(ETherm)) HADiscovery(BR_OUTSIDE_SENSOR_TOPIC, "\0", OUTSIDE_TEMPERATURE_E, BOILERROOM_HA_SENSOR_TOPIC, "temperature");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_E, dEThermometerS, mqtt_HAClass_temperature);
    if (!check_isValidTemp(WTherm)) HADiscovery(BR_OUTSIDE_SENSOR_TOPIC, "\0", OUTSIDE_TEMPERATURE_W, BOILERROOM_HA_SENSOR_TOPIC, "temperature");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_W, dWThermometerS, mqtt_HAClass_temperature);
    if (!check_isValidTemp(STherm)) HADiscovery(BR_OUTSIDE_SENSOR_TOPIC, "\0", OUTSIDE_TEMPERATURE_S, BOILERROOM_HA_SENSOR_TOPIC, "temperature");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_S, dSThermometerS, mqtt_HAClass_temperature);
    if (!check_isValidTemp(OutsideTempAvg)) HADiscovery(BR_OUTSIDE_SENSOR_TOPIC, "\0", OUTSIDE_TEMPERATURE_A, BOILERROOM_HA_SENSOR_TOPIC, "temperature");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_A, dallThermometerS, mqtt_HAClass_temperature);
    if (!check_isValidTemp(coTherm)) HADiscovery(BOILERROOM_SENSOR_TOPIC, "\0", HEATERCO_TEMPERATURE, BOILERROOM_HA_SENSOR_TOPIC, "temperature");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, HEATERCO_TEMPERATURE, dcoThermstat, mqtt_HAClass_temperature);
    if (!check_isValidTemp(waterTherm)) HADiscovery(BOILERROOM_SENSOR_TOPIC, "\0", WATER_TEMPERATURE, BOILERROOM_HA_SENSOR_TOPIC, "temperature");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, WATER_TEMPERATURE, dwaterThermstat, mqtt_HAClass_temperature);
    if (!check_isValidTemp(bmTemp)) HADiscovery(BOILERROOM_SENSOR_TOPIC, "\0", BOILERROOM_TEMPERATURE, BOILERROOM_HA_SENSOR_TOPIC, "temperature");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_TEMPERATURE, dbmtemperature, mqtt_HAClass_temperature);
    //pressure
    HADiscovery(BOILERROOM_SENSOR_TOPIC, "\0", BOILERROOM_PRESSURE, BOILERROOM_HA_SENSOR_TOPIC, "pressure");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PRESSURE, dbmpressure, mqtt_HAClass_pressure);
    //high
    HADiscovery(BOILERROOM_SENSOR_TOPIC, "\0", BOILERROOM_HIGH, BOILERROOM_HA_SENSOR_TOPIC, "high");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_HIGH, dbmhigh, mqtt_HAClass_high);
    HADiscovery(BOILERROOM_SENSOR_TOPIC, "\0", BOILERROOM_HIGHREAL, BOILERROOM_HA_SENSOR_TOPIC, "high");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_HIGHREAL, dbmhighr, mqtt_HAClass_high);
    //CO ppm
    HADiscovery(BOILERROOM_SENSOR_TOPIC, "\0", BOILERROOM_COVAL, BOILERROOM_HA_SENSOR_TOPIC, "co");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_COVAL, dcoS, mqtt_HAClass_co);
    //pumps energy
    // mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP1WA_E, dpump1energyS, mqtt_HAClass_energy);
    // mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP2CO_E, dpump2energyS, mqtt_HAClass_energy);

    HADiscovery(BR_MEDIA_SENSOR_TOPIC, "\0", BOILERROOM_PUMP1WA_E, BOILERROOM_HA_SENSOR_TOPIC, "energy");   // mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP1WA_E, dpump1energyS, mqtt_HAClass_energy);
    HADiscovery(BR_MEDIA_SENSOR_TOPIC, "\0", BOILERROOM_PUMP2CO_E, BOILERROOM_HA_SENSOR_TOPIC, "energy");   //mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP2CO_E, dpump2energyS, mqtt_HAClass_energy);
    //pumps switch/state
    HADiscovery(BOILERROOM_SWITCH_TOPIC, "\0", BOILERROOM_PUMP1WA, BOILERROOM_HA_SWITCH_TOPIC, "switch");   //mqttHAPublish_Config(BOILERROOM_HA_SWITCH_TOPIC, BOILERROOM_SWITCH_TOPIC, BOILERROOM_PUMP1WA, dpump1, mqtt_HAClass_switch, BOILERROOM_SWITCH_TOPIC_SET);
    HADiscovery(BOILERROOM_SWITCH_TOPIC, "\0", BOILERROOM_PUMP2CO, BOILERROOM_HA_SWITCH_TOPIC, "switch");   //mqttHAPublish_Config(BOILERROOM_HA_SWITCH_TOPIC, BOILERROOM_SWITCH_TOPIC, BOILERROOM_PUMP2CO, dpump2, mqtt_HAClass_switch, BOILERROOM_SWITCH_TOPIC_SET);
  }
    log_message((char*)F("MQTT Data Sended..."));
}
#endif
































#ifdef ENABLE_INFLUX
void updateInfluxDB()
{
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
  InfluxSensor.clearFields();
  // Report RSSI of currently connected network
  InfluxSensor.addField(me_lokalizacja + "_" + "rssi", (WiFi.RSSI()));
  InfluxSensor.addField(me_lokalizacja + "_" + "CRT",  (CRTrunNumber));
  InfluxSensor.addField(me_lokalizacja + "_" + "uptime",  ((millis())/1000));   //w sekundach

  if (bmTemp != InitTemp) {InfluxSensor.addField(me_lokalizacja + "_" + String(BOILERROOM_TEMPERATURE), bmTemp);}
  InfluxSensor.addField(me_lokalizacja + "_" + String(BOILERROOM_PRESSURE), dbmpressval);
  InfluxSensor.addField(me_lokalizacja + "_" + String(BOILERROOM_HIGH), bm_high);
  InfluxSensor.addField(me_lokalizacja + "_" + String(BOILERROOM_HIGHREAL), bm_high_real);
  InfluxSensor.addField(me_lokalizacja + "_" + String(BOILERROOM_COVAL), dcoval);
  if (coTherm != InitTemp) {InfluxSensor.addField(me_lokalizacja + "_" + String(HEATERCO_TEMPERATURE), coTherm);}
  if (waterTherm != InitTemp) {InfluxSensor.addField(me_lokalizacja + "_" + String(WATER_TEMPERATURE), waterTherm);}
  InfluxSensor.addField(me_lokalizacja + "_" + String(BOILERROOM_PUMP1WA), prgstatusrelay1WO?"1":"0");
  InfluxSensor.addField(me_lokalizacja + "_" + String(BOILERROOM_PUMP1WA_E), energy1used);
  InfluxSensor.addField(me_lokalizacja + "_" + String(BOILERROOM_PUMP2CO_E), energy2used);
    InfluxSensor.addField(me_lokalizacja + "_" + String(BOILERROOM_PUMP2CO), prgstatusrelay2CO?"1":"0");
  if (NTherm != InitTemp) {InfluxSensor.addField(me_lokalizacja + "_" + String(OUTSIDE_TEMPERATURE_N), NTherm);}
  if (ETherm != InitTemp) {InfluxSensor.addField(me_lokalizacja + "_" + String(OUTSIDE_TEMPERATURE_E), ETherm);}
  if (WTherm != InitTemp) {InfluxSensor.addField(me_lokalizacja + "_" + String(OUTSIDE_TEMPERATURE_W), WTherm);}
  if (STherm != InitTemp) {InfluxSensor.addField(me_lokalizacja + "_" + String(OUTSIDE_TEMPERATURE_S), STherm);}
  if (OutsideTempAvg != InitTemp) {InfluxSensor.addField(me_lokalizacja + "_" + String(OUTSIDE_TEMPERATURE_A), OutsideTempAvg);}

  // Print what are we exactly writing
  // don't why thi schanges isadslinitialised to true    sprintf(log_chars,"Writing to InfluxDB: %s", String(InfluxClient.pointToLineProtocol(InfluxSensor)).c_str());
  //log_message(log_chars); //(char*)F("Writing to InfluxDB: "));
  // Write point
  if (!InfluxClient.writePoint(InfluxSensor))
  {
    sprintf(log_chars,"InfluxDB write failed: %s", String(InfluxClient.getLastErrorMessage()).c_str());
  }
}
#endif

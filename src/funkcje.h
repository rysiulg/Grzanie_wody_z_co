
void array_to_string(byte array[], unsigned int len, char buffer[])
{
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
}

void hw_wdt_disable(){
  *((volatile uint32_t*) 0x60000900) &= ~(1); // Hardware WDT OFF
}

void hw_wdt_enable(){
  *((volatile uint32_t*) 0x60000900) |= 1; // Hardware WDT ON
}

void dumpByteArray(const byte * byteArray) {
#ifdef debug
  const byte arraySize = sizeof(byteArray)*2;
  for (int i = 0; i < arraySize; i++)
  {
    Serial.print("");  //0x
    if (byteArray[i] < 0x10)
      Serial.print("0");
    Serial.print(byteArray[i], HEX);
 //   Serial.print(", ");
  }
  Serial.println("");
#endif
}
byte nibble(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;  // Nmqttident a valid hexadecimal character
}

void hexCharacterStringToBytes(byte *byteArray, const char *hexString) {
  bool oddLength = strlen(hexString) & 1;

  byte currentByte = 0;
  byte byteIndex = 0;

  for (byte charIndex = 0; charIndex < strlen(hexString); charIndex++)
  {
    bool oddCharIndex = charIndex & 1;

    if (oddLength)
    {
      // If the length is odd
      if (oddCharIndex)
      {
        // odd characters go in high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        // Even characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
    else
    {
      // If the length is even
      if (!oddCharIndex)
      {
        // Odd characters go into the high nibble
        currentByte = nibble(hexString[charIndex]) << 4;
      }
      else
      {
        // Odd characters go into low nibble
        currentByte |= nibble(hexString[charIndex]);
        byteArray[byteIndex++] = currentByte;
        currentByte = 0;
      }
    }
  }
}

//******************************************************************************************
void saveConfigCallback() {
#ifdef debug
  Serial.println("Should save config");
#endif
  shouldSaveConfig = true;
  SaveConfig();
  SaveEnergy();
  #ifdef debug
    Serial.println("SaveConfigCallback saved configxxxxxxxxxxxxxxxxxxxxxx");
  #endif
}

String checkUnassignedSensors() {
  String nowe18baddr = F("<p>New unassigned:<br>");
  nowe18baddr += UnassignedTempSensor;
  nowe18baddr += "<br></p>";
  if (UnassignedTempSensor == "") nowe18baddr = "\0";
  return nowe18baddr;
}


//******************************************************************************************
//     background-color: #01DF3A;
#ifndef enableWebSocket
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <title>%me_lokalizacja%</title>
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">

  <style>
    %stylesectionadd%
  </style>
</head>
<body>
  <h2>%ver%</h2>
  <p>
  <sup class="units">Uptime <b><span id="%uptime%">%uptimewart%</span></B></sup>
  <br/>
<sup class="units">%dane%</sup>
  </p>
   <span id="%do_stopkawebsiteS%" class="units">%stopkawebsite%</span>

   %bodywstaw%
   %bodywstaw1%
   %bodywstaw2%
  <p>
    %stopkawebsite0%
  </p>
</body>
<script>
  %scriptsectionreplace%
  %scriptsectionreplace1%
  %scriptsectionreplace2%
</script>
</html>)rawliteral";
#endif
//******************************************************************************************
String getpumpstatus(uint8_t pompa) {

  if (pompa == 1) {
    String ptr;
    if (!prgstatusrelay1WO or pump1energyLast < pumpmincurrent) {  //digitalRead(relay1) == HIGH
      ptr = F("<B>Stoi</B> ");
      ptr += getkWh(pump1energyLast);
      ptr += F("W</B><BR>");
      if (najpierwCO) {
        ptr += F("<font color=\"red\"><sup class='units'>Zmiana priorytetu na CO: ");
        ptr += String(najpierwCO);
        ptr += F("</sup></font>");
      }
    } else {
      ptr = F("<B><font color=\"red\">Chodzi</font> ");
      ptr += getkWh(pump1energyLast);
      ptr += F("W</B><BR>");
      if (najpierwCO) {
        ptr += F("<font color=\"red\"><sup class='units'>Zmiana priorytetu na CO: ");
        ptr += String(najpierwCO);
        ptr += F("</sup></font>");
      }
    } return String(ptr);
  } else {
    String ptr;
    if (!prgstatusrelay2CO or pump2energyLast < pumpmincurrent) {  //digitalRead(relay2) == HIGH
      ptr = F("<B>Stoi</B> ");
      ptr += getkWh(pump2energyLast);
      ptr += F("W</B><BR>");
      ptr += F("<sup class='units'>histereza: ");
      ptr += String(histereza);
      ptr += F("</sup>");
    } else {
      ptr = F("<B><font color=\"red\">Chodzi</font> ");
      ptr += getkWh(pump2energyLast);
      ptr += F("W</B><BR> <sup class='units'>histereza: ");
      ptr += String(histereza);
      ptr += F("</sup>");
    } return String(ptr);
  }
  return "";
}
// Replaces placeholder with DHT values
String processor(const String var) {
  #ifndef enableWebSocket
  #ifdef debug
  sprintf(log_chars,"Processing processor: %s",var.c_str());
  log_message(log_chars);
  #endif
  #ifdef wdtreset
  wdt_reset();
  #endif
  if (var == "ver") {
    String a = "</B>ESP CO Server dla: <B>" + String(me_lokalizacja) + "</B><BR>v. ";
    a += me_version;
    a += "<br><font size=\"2\" color=\"DarkGreen\">";
    #ifdef enableMQTT
    a += espClient.connected()? "MQTT "+String(msg_Connected)+": "+String(mqtt_server)+":"+String(mqtt_port) : "MQTT "+String(msg_disConnected)+": "+String(mqtt_server)+":"+String(mqtt_port) ;  //1 conn, 0 not conn
    #endif
    #ifdef ENABLE_INFLUX
    a += " + INFLUXDB: "+String(INFLUXDB_DB_NAME)+"/"+String(InfluxMeasurments);
    #endif
    a += "</font>";
    return a;
  }
  if (var == "scriptsectionreplace") {
    String ptr;
    String tmp;
    const String function0 = "setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\"";
    const String function1 = "\").innerHTML=this.responseText)},e.open(\"GET\",\"/";
    const String function2 = "\",!0),e.send()},";
    const String function3 = ");\n";
    unsigned long int step=125;
    unsigned long int refreshtime = 9100;
    ptr=F("function uTI(e,n){document.getElementById(n).value=e};\n");
    tmp=String(uptimelink);
    refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime/2) +function3;

    tmp=String(dbmtemperature); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    tmp=String(dbmpressure); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    tmp=String(dbmhigh); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    tmp=String(dbmhighr); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    return ptr;
  }
  if (var == "scriptsectionreplace1") {
    unsigned long int step=125*2;
    unsigned long int refreshtime = 9100;
    String ptr ="\0";
    String tmp;
    const String function0 = "setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\"";
    const String function1 = "\").innerHTML=this.responseText)},e.open(\"GET\",\"/";
    const String function2 = "\",!0),e.send()},";
    const String function3 = ");\n";
    tmp=String(dcoS); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    tmp=String(dcoThermstat); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    tmp=String(dwaterThermstat); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    tmp=String(dNThermometerS); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    tmp=String(dWThermometerS); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    return ptr;
  }
  if (var == "scriptsectionreplace1") {
    unsigned long int step=125*2;
    unsigned long int refreshtime = 9100;
    const String function0 = "setInterval(function(){var e=new XMLHttpRequest;e.onreadystatechange=function(){4==this.readyState&&200==this.status&&(document.getElementById(\"";
    const String function1 = "\").innerHTML=this.responseText)},e.open(\"GET\",\"/";
    const String function2 = "\",!0),e.send()},";
    const String function3 = ");\n";
    String ptr;
    ptr=F("function uTI(e,n){document.getElementById(n).value=e};\n");
    String tmp;
    tmp=String(dEThermometerS); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    refreshtime+=step;
    tmp=String(dSThermometerS); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    tmp=String(dallThermometerS); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    tmp=String(do_stopkawebsiteS); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;

    //ptr+="if(document.getElementById(\"blink\")){var blink=document.getElementById('blink');setInterval(function(){blink.style.color=(blink.style.color=='red'?'blue':'red');}, 1200);}"; //blink
    ptr+=F("function bt(){document.querySelectorAll('.blink').forEach(e =>{setInterval(()=>{console.log(e);e.style.color=(blink.style.color=='red'?'blue':'red')},500);});}document.addEventListener('DOMContentLoaded',()=>{bt();});"); //classList.toggle('hide');
    Serial.println(F("scriptsectionreplace: ")+String(ptr.length()));
    return String(ptr);
  }
  if (var == "dane") {
    String a = "Raport dla Hosta: <B>";  //PrintHex8c(GUID,0x0,sizeof(GUID)/sizeof(GUID[0]));
//    a += me_lokalizacja;
//    a += "</B>&nbsp;&nbsp;&nbsp;
    a = "MAC: <B>";
    #ifdef debug
      Serial.println(String(millis())+F(": www dane"));
    #endif
    a += PrintHex8(mac, ':', sizeof(mac) / sizeof(mac[0]));
    a += F("</b>&nbsp;&nbsp; Ro: <B>");
    a += Ro;
    a += F("</B>,&nbsp;&nbsp;WiFi (RSSI): <B>");
    a += WiFi.RSSI();
    a += F("dBm</b> CRT:");
    a += String(runNumber);
    a += F("<br> Energy 1 used WO: <b>");
    a += String(energy1used,4);
    a += F("kWh</b>, Energy 2 used CO: <b>");
    a += String(energy2used,4);
    a += F("kWh</b><br> ADS initialised: <b>");
    a += String(isadslinitialised ? "Tak" : "Nie");
    a += F("</B>");
    if (String(checkUnassignedSensors()).length()>0) a += F("<br> Unassigned: ") + String(checkUnassignedSensors());
    if (ownother.length()>0) { a += F("<br> Własny string: ")+ String(ownother); }
    return String(a);
  }
  if (var == "uptime") {
    return String(uptimelink);
  }
  if (var == "uptimewart") {
    return String(uptimedana(0)); //0 bo czas startu od milis
  }
  if (var == "me_lokalizacja") {
    return String(me_lokalizacja);
  }
  if (var == "stylesectionadd") {
    String ptr;
    ptr=F("html{font-family:Arial;display:inline-block;margin:0 auto;text-align:center}\
    h2{font-size:2.1rem}\
    p{font-size:1.9rem}\
    .units{font-size:1.1rem}\
    .dht-labels{font-size:1.3rem;vertical-align:middle;padding-bottom:6px}\
    .dht-labels-temp{font-size:3.3rem;font-weight:700;vertical-align:middle;padding-bottom:6px}\
    table,td,th{border-color:green;border-collapse:collapse;border-style:outset;margin-left:auto;margin-right:auto;border:0;text-align:center;padding-left: 5px;padding-right: 10px;padding-top: 5px;padding-bottom: 10px;}\
    input{margin:.4rem}\
    td{height:auto;width:auto}");
    ptr+=F("body{background-color:lightyellow;}");
    ptr+=F("#blink{font-weight:bold;font-size:20px;font-family:sans-serif;transition: 0.5s;}");//         color: #2d38be;   font-size: 20px; font-weight: bold; color: #2d38be;
    return ptr;
  }
  // if (var == "stopkawebsite") {
  //   return do_stopkawebsite();
  // }
  if (var == "stopkawebsite0") {
    String ptr;
      ptr =  F("<p><span class=\"units\" id=\"links\">")+String(getlinki())+F("</span></p>");
      ptr += F("<p><br><span class='units'><a href='")+String(update_path)+F("' target=\"_blank\">")+String(Update_web_link)+F("</a> &nbsp; &nbsp;&nbsp;");
      #ifdef enableWebSerial
      ptr += "<a href='/webserial' target=\"_blank\">"+String(Web_Serial)+"</a>&nbsp;";
      #endif
      ptr += F("<br>&copy; ");
      ptr += stopka;
    return ptr;
  }
  if (var == "bodywstaw") {
    String ptr =F("\0");
    ptr=F("<form action=\"/get\"><table>");
//    ptr+="<table>";
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_NEWS)+F("</span><B><span class=\"dht-labels-temp\" id=\"")+String(dallThermometerS)+F("\">&nbsp;<font color=\"Green\">")+(OutsideTempAvg==InitTemp?"--.-":String(OutsideTempAvg))+F("</font></span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("<tr><td>");
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_COHeat)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dcoThermstat)+F("\">&nbsp;<font color=\"Blue\">")+(coTherm==InitTemp?"--.-":String(coTherm,1))+F("</font></span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("<td></td");
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_WaterCOHeat)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dwaterThermstat)+F("\">&nbsp;<font color=\"blue\">")+(waterTherm==InitTemp?"--.-":String(waterTherm,1))+F("</font></span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td></tr>");
    ptr+=F("<tr><td>");
    ptr+="<p>"+ppmicon+F("<span class=\"dht-labels\">")+String(pump)+F(" 1 ")+String(CO_heat)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dpump1)+F("\">&nbsp;")+String(getpumpstatus(1))+F("</span><sup class=\"units\"> </sup></B></p>");
    ptr+=F("<td></td");
    ptr+="<p>"+ppmicon+F("<span class=\"dht-labels\">")+String(pump)+F(" 2 ")+String(water)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dpump2)+F("\">&nbsp;")+String(getpumpstatus(2))+F("</span><sup class=\"units\"> </sup></B></p>");
    ptr+=F("</td></tr>");
    return ptr;
  }
if (var == "bodywstaw1") {
    String ptr ="\0";
    //if (NTherm==InitTemp) wart="--.--"; else wart=NTherm;
    ptr+=F("<tr><td>");
    ptr+=F("<p>")+attiicon+F("<span class=\"dht-labels\">")+String(Attitude)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dbmhigh)+F("\">&nbsp;")+String(bm_high,0)+F("</span><sup class=\"units\">mnpm</sup></B></p>");
    ptr+=F("<td></td");
    ptr+=F("<p>")+attiicon+F("<span class=\"dht-labels\">")+String(Attitude_real)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dbmhighr)+F("\">&nbsp;")+String(bm_high_real,0)+F("</span><sup class=\"units\">mnpm</sup></B></p>");
    ptr+=F("</td></tr>");
    ptr+=F("<tr><td>");
    ptr+=F("<p>")+presicon+F("<span class=\"dht-labels\">")+String(Pressure)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dbmpressure)+F("\">&nbsp;")+String(dbmpressval)+F("</span><sup class=\"units\">hPa</sup></B></p>");
    ptr+=F("<td></td");
    ptr+=F("<p>")+ppmicon+F("<span class=\"dht-labels\">")+String(GAS_CO)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dcoS)+F("\">&nbsp;")+String(dcoval)+F("</span><sup class=\"units\">ppm</sup></B></p>");
    ptr+=F("</td></tr>");
    ptr+=F("<tr><td>");
    //if (NTherm<-100 or NTherm>100) wart="-.--"; else wart=String(NTherm);
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_N)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dNThermometerS)+F("\">&nbsp;")+(NTherm==InitTemp?"--.-":String(NTherm,1))+F("</span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td><td>");
    //if (ETherm<-100 or ETherm>100) wart="-.--"; else wart=String(ETherm);
    ptr+="<p>"+tempicon+F("<span class=\"dht-labels\">")+String(Temp_E)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dEThermometerS)+F("\">&nbsp;")+(ETherm==InitTemp?"--.-":String(ETherm,1))+F("</span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td></tr><tr><td>");
    return ptr;
  }
if (var == "bodywstaw2") {
    String ptr =F("\0");
    //if (WTherm<-100 or WTherm>100) wart="-.--"; else wart=String(WTherm);
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_W)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dWThermometerS)+F("\">&nbsp;")+(WTherm==InitTemp?"--.-":String(WTherm,1))+F("</span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td><td>");
    //if (STherm<-100 or STherm>100) wart="-.--"; else wart=String(ETherm);
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_S)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dSThermometerS)+F("\">&nbsp;")+(STherm==InitTemp?"--.-":String(STherm,1))+F("</span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td></tr>");
    ptr+=F("<tr><td>");
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_BoilerRoom)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dbmtemperature)+F("\">&nbsp;")+String(bmTemp,1)+F("</span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td></tr>");
    ptr+=F("</table></form>");
    #ifdef debug1
    Serial.print(String(millis())+F(": www BODYWSTAW len: "));
    Serial.println(ptr.length());
    #endif
    return ptr;
  }


  #ifdef debug
    Serial.print(F("End processor "));
//    Serial.println(var);
  #endif
  return String("\0");
  #endif
}

String do_stopkawebsite() {
      String ptr;
      ptr = F("&nbsp;");
      // if (room_temp[8].switch_state) {
      //   ptr += "<i class='fas fa-fire' style='color: red'></i>"; ptr += "<span class='dht-labels'>"+String(Flame_Active_Flame_level)+"</span><B>"+ String("flame_level",0)+"<sup class=\"units\">&#37;</sup></B>";
      //   ptr += "<br>";
      // }
      if (prgstatusrelay1WO) ptr += F("<font color=\"red\"><span class='dht-labels'><B>")+String(BOILER_IS_HEATING)+F("<br></B></span></font>");
      if (prgstatusrelay2CO) ptr += F("<font color=\"blue\"><span class='dht-labels'><B>")+String(Second_Engine_Heating_PompActive)+F("<br></B><br></span></font>");


      // if (status_Fault) ptr += "<span class='dht-labels'><B>!!!!!!!!!!!!!!!!! status_Fault !!!!!!!<br></B></span>";

      // if (status_CHActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>"+String(BOILER_IS_HEATING)+"<br></B></span></font>";
      // if (enableHotWater) ptr += "<span class='dht-labels'><B>"+String(DHW_HEAT_ON)+"<br></B></span>";
      // if (status_WaterActive) ptr += "<font color=\"red\"><span class='dht-labels'><B>"+String(Boiler_Active_heat_DHW)+"<br></B></span></font>";
      // if (status_Cooling) ptr += "<font color=\"orange\"><span class='dht-labels'><B>"+String(CoolingMode)+"<br></B></span></font>";
      // if (status_Diagnostic) ptr += "<font color=\"darkred\"><span class='dht-labels'><B>"+String(DiagMode)+"<br></B></span></font>";

//      if (flame_time>0) ptr+= "<font color=\"green\"><span class='dht-labels'>"+String(Flame_time)+"<B>"+uptimedana(millis()-flame_time)+"<br></B><br></span></font>";
//      ptr += "<br>"+String(Flame_total)+"<B>"+String(flame_used_power_kwh,4)+"kWh</B>";
    return String(ptr).c_str();
}
//******************************************************************************************

String getlinki() {
  String ptr;
  wdt_reset();
  #ifdef debug
//      Serial.print(F("GetLinki: "));
//      Serial.println(ptr);
  #endif
  ptr = F("\0");
  if (checkUnassignedSensors().length()>0) {
    ptr += F("<br><sup class='units'>");
    ptr += String(checkUnassignedSensors()); //UnassignedSensors;
    ptr += F("</sup>");
  }
  ptr += F("<br>wymusza pompe CO poniżej temperatury średniej zewnetrznej: &nbsp;");
  ptr += String(forceCObelow);
  ptr += F("<br>Temperatura graniczna na wymienniku oznacza ze piec sie grzeje: &nbsp;");
  ptr += String(coConstTempCutOff);

  #ifdef debug
 //     Serial.print(F("Endlinki: "));
 //     Serial.println(ptr);
    #endif
  return ptr.c_str();
}

void starthttpserver() {
  #ifndef enableWebSocket
  //  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  //  httpwebserver.on("/test", HTTP_GET, []() {
  //    httpwebserver.send(200, "text/html",  SendHTML(dcoThermstatometerS->getlast(), waterThermometerS->getlast())); //dcoThermstat,waterTherm));
  //    httpwebserver.sendHeader("Connection", "close");
  //  });
  #ifdef wdtreset
  wdt_reset();
  hw_wdt_disable();
  #endif
  #ifdef debug
    Serial.println("http Server Initialization...");
  #endif
//  webserver.setAuthentication("","");

  webserver.on(slashstr uptimelink , HTTP_GET, [](AsyncWebServerRequest * request) {
  //  request.setAuthentication("", "");
    request->send(200, "text/plain; charset=utf-8", (String)uptimedana(0));  //0 czas startu
  }).setAuthentication("", "");
  webserver.on(slashstr dbmtemperature, HTTP_GET, [](AsyncWebServerRequest * request) {  //webserver.on(strcat(slashstr,dbmtemperature), HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200, "text/plain; charset=utf-8", String(bmTemp,1));
  }).setAuthentication("", "");
  webserver.on(slashstr dbmpressure, HTTP_GET, [](AsyncWebServerRequest * request) {  //webserver.on(strcat(slashstr,dbmpressure), HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200, "text/plain; charset=utf-8", String(dbmpressval));
  }).setAuthentication("", "");
  webserver.on(slashstr dbmhigh, HTTP_GET, [](AsyncWebServerRequest * request) {  //webserver.on(strcat(slashstr,dbmhigh), HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200, "text/plain; charset=utf-8", String(bm_high,0));
  }).setAuthentication("", "");
  webserver.on(slashstr dbmhighr, HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200, "text/plain; charset=utf-8", String(bm_high_real,0));
  }).setAuthentication("", "");
  webserver.on(slashstr dcoS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(dcoval));
  }).setAuthentication("", "");
  webserver.on(slashstr dcoThermstat, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(F("<font color=\"Blue\">"))+(coTherm==InitTemp?"--.-":String(coTherm,1))+String(+F("<font>"))); //dcoThermstatometerS->getlast());
  }).setAuthentication("", "");
  webserver.on(slashstr dwaterThermstat, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(F("<font color=\"Blue\">"))+(waterTherm==InitTemp?"--.-":String(waterTherm,1))+String(+F("<font>")));
  }).setAuthentication("", "");
  webserver.on(slashstr dpump1, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(getpumpstatus(1)));
  }).setAuthentication("", "");
  webserver.on(slashstr dpump2, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(getpumpstatus(2)));
  }).setAuthentication("", "");
  webserver.on(slashstr dNThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", (NTherm==InitTemp?"--.-":String(NTherm,1)));
  }).setAuthentication("", "");
  webserver.on(slashstr dWThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", (WTherm==InitTemp?"--.-":String(WTherm,1)));
  });
  webserver.on(slashstr dEThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", (ETherm==InitTemp?"--.-":String(ETherm,1)));
  }).setAuthentication("", "");
  webserver.on(slashstr dSThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", (STherm==InitTemp?"--.-":String(STherm,1)));
  });
  webserver.on(slashstr dallThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(F("<font color=\"Green\">"))+(OutsideTempAvg==InitTemp?"--.-":String(OutsideTempAvg))+String(+F("<font>")));
  }).setAuthentication("", "");
  webserver.on(slashstr do_stopkawebsiteS , HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html; charset=utf-8", String(do_stopkawebsite()));
  }).setAuthentication("", "");
  webserver.on(slashstr , HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html; charset=utf-8",  index_html, processor); //,processor
  }).setAuthentication("", "");
  webserver.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
//    AsyncWebHandler->setAuthentication("", "");
    handleUpdate(request);
  }).setAuthentication("", "");
  webserver.on("/doUpdate", HTTP_POST, [](AsyncWebServerRequest * request) {},
  [](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data,
     size_t len, bool final) {
    handleDoUpdate(request, filename, index, data, len, final);
  }
  ).setAuthentication("", "");

  webserver.onNotFound(notFound);
  webserver.begin();
  #endif
}

#ifndef enableWebSocket
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}
#endif

//******************************************************************************************
void display_temp_rotation() {
  //return;
  switch (what_display) {
    case 0 : {
        display.print((String)"CO  " + int(coTherm * 100));              // display LOOP on the display
#ifdef debug
        Serial.println((String)"CO  " + int(coTherm * 100));
#endif
        //    debugA("CO: %f", float(coTherm * 1));
        if (what_display == last_case - 1)  what_display = 0; else what_display++;
        break;
      }
    case 1 : {
        display.blink();
        display.print((String)"WODA" + int(waterTherm * 100));              // display LOOP on the display
#ifdef debug
        Serial.println((String)"WODA" + int(waterTherm * 100));
#endif
        //  debugA("WODA: %f", float(waterTherm));
        if (what_display == last_case - 1)  what_display = 0; else what_display++; //start new
        break;
      }
      case 2 : {
        if (forceWater==true) {
          display.blink();
          display.print((String)"dAJ dH2O" );              // display LOOP on the display
#ifdef debug
          Serial.println((String)"dAJ dH2O from rmqttidentation");
#endif
        }
        if (forceCO==true) {
          display.blink();
          display.print((String)"dAJ dCO " );              // display LOOP on the display
#ifdef debug
          Serial.println((String)"dAJ dH2O from rmqttidentation");
#endif
        }
        if (what_display == last_case - 1)  what_display = 0; else what_display++;
        break;
      }
    case 3 : {
        display.print((String)"CIS   " + int(dbmpressval));
#ifdef debug
        Serial.println((String)"CIS   " + int(dbmpressval));
#endif
        if (what_display == last_case - 1)  what_display = 0; else what_display++;
        break;
      }
    case 4 : {
        String wyniktemp;
        int aver=OutsideTempAvg*10;
        if (abs(aver)==0) wyniktemp="00";
        if (abs(aver)<10 and abs(aver)>0) {
          if (aver<0) wyniktemp="-0"+(String)(abs(aver)); else wyniktemp="0"+(String)aver;
        }
        if (abs(aver)<100 and abs(aver)>=10) {
          if (aver<0) wyniktemp="-"+(String)(abs(aver)); else wyniktemp=""+(String)aver;
        }
        display.print((String)"POLE   " + wyniktemp);

#ifdef debug
        Serial.println((String)"POLE TEMP:" + int(OutsideTempAvg*100));
#endif
        if (what_display == last_case - 1)  what_display = 0; else what_display++;
        break;
      }
    case (last_case-1) : {
        display.print((String)"GAS " + int(dcoval));
#ifdef debug
        Serial.println((String)"GAS " + int(dcoval));
        Serial.println("Millis: " + String(millis()));
#endif
        if (what_display == last_case - 1)  what_display = 0; else what_display++;
        break;
      }
  }
}



/****************** MQResistanceCalculation ****************************************
  Input:   raw_adc - raw value read from adc, which represents the voltage
  Output:  the calculated sensor resistance
  Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE * (1023 - raw_adc) / raw_adc));
}

/***************************** MQCalibration ****************************************
  Input:   mq_pin - analog channel
  Output:  Ro of the sensor
  Remarks: This function assumes that the sensor is in clean air. It use
         MQResistanceCalculation to calculates the sensor resistance in clean air
         and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about
         10, which differs slightly between different sensors.
************************************************************************************/
float MQCalibration(int mq_pin)
{
  int i;
  float val = 0;

  for (i = 0; i < CALIBARAION_SAMPLE_TIMES; i++) {      //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
    #ifdef wdtreset
    wdt_reset();
    #endif
  }
  val = val / CALIBARAION_SAMPLE_TIMES;                 //calculate the average value
  val = val / RO_CLEAN_AIR_FACTOR;                      //divided by RO_CLEAN_AIR_FACTOR yields the Ro
  //according to the chart in the datasheet
  return val;
}
/*****************************  MQRead *********************************************
  Input:   mq_pin - analog channel
  Output:  Rs of the sensor
  Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/
float MQRead(int mq_pin)
{
  int i;
  float rs = 0;

  for (i = 0; i < READ_SAMPLE_TIMES; i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs / READ_SAMPLE_TIMES;

  return rs;
}

/*****************************  MQGetGasPercentage **********************************
  Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
  Output:  ppm of the target gas
  Remarks: This function passes different curves to the MQGetPercentage function which
         calculates the ppm (parts per million) of the target gas.
************************************************************************************/
//int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
//{
//  if ( gas_id == GAS_H2) {
//    return MQGetPercentage(rs_ro_ratio, H2Curve);
//  }
//  return 0;
//}

/*****************************  MQGetPercentage **********************************
  Input:   rs_ro_ratio - Rs divided by Ro
         pcurve      - pointer to the curve of the target gas
  Output:  ppm of the target gas
  Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm)
         of the line could be derived if y(rs_ro_ratio) is provided. As it is a
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic
         value.
************************************************************************************/
int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10, ( ((log(rs_ro_ratio) - pcurve[1]) / pcurve[2]) + pcurve[0])));
}

void gas_leak_check() {
  if (dcoval > gas_leakage) {
    digitalWrite(buzzer, HIGH);
#ifdef debug
    Serial.print("Gas leakage: ");
    Serial.print(dcoval);
    Serial.println("ppm");
#endif
    //      debugA("Gas leakage... steżenie: %d ppm",gasCOMeterS->getlast());
  }
  else
  {
    digitalWrite(buzzer, LOW);
#ifdef debug
    //    Serial.print("Gas nmqttident leak: ");
    //    Serial.print(gasCOMeterS->getlast());
    //    Serial.println("ppm");
#endif
    //      debugA("Gas leakage... steżenie: %d ppm",gasCOMeterS->getlast());
  }
}
bool loadConfig() {
  // is it correct?
  if (sizeof(CONFIGURATION)<1024) EEPROM.begin(1024); else EEPROM.begin(sizeof(CONFIGURATION)+128); //Size can be anywhere between 4 and 4096 bytes.
  EEPROM.get(1,runNumber);
  if (isnan(runNumber)) runNumber=0;
  runNumber++;
  EEPROM.get(1+sizeof(runNumber),energy1used);
  if (isnan(energy1used)) energy1used = 0;
  EEPROM.get(1+sizeof(runNumber)+sizeof(energy1used),energy2used);
  if (isnan(energy2used)) energy2used = 0;
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2] &&
      EEPROM.read(CONFIG_START + 3) == CONFIG_VERSION[3] &&
      EEPROM.read(CONFIG_START + 4) == CONFIG_VERSION[4]){

  // load (overwrite) the local configuration struct
    for (unsigned int i=0; i<sizeof(configuration_type); i++){
      *((char*)&CONFIGURATION + i) = EEPROM.read(CONFIG_START + i);
    }
    strcpy(ssid, CONFIGURATION.ssid);
    strcpy(pass, CONFIGURATION.pass);
    strcpy(mqtt_server, CONFIGURATION.mqtt_server);
    strcpy(mqtt_user, CONFIGURATION.mqtt_user);
    strcpy(mqtt_password, CONFIGURATION.mqtt_password);
    mqtt_port = CONFIGURATION.mqtt_port;
    najpierwCO = CONFIGURATION.najpierwCO;
    if (najpierwCO == true) waitCOStartingmargin=millis();
    coConstTempCutOff = CONFIGURATION.coConstTempCutOff;
    forceCObelow = CONFIGURATION.forceCObelow;
    histereza = CONFIGURATION.histereza;
    forceWater = CONFIGURATION.forceWater;
    forceCO = CONFIGURATION.forceCO;

    return true; // return 1 if config loaded
  }
  //try get only my important values

  return false; // return 0 if config NOT loaded
}
void SaveConfig() {
  //EEPROM.put(1, runNumber);
  // EEPROM.put(1+sizeof(runNumber), energy1used);
  // EEPROM.put(1+sizeof(runNumber)+sizeof(energy1used), energy2used);

  log_message((char*)F("Saving config...........................prepare"));
  SaveEnergy();
  unsigned int temp =0;
  //firs read content of eeprom
  EEPROM.get(1,temp);
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2] &&
      EEPROM.read(CONFIG_START + 3) == CONFIG_VERSION[3] &&
      EEPROM.read(CONFIG_START + 4) == CONFIG_VERSION[4]){
  // load (overwrite) the local configuration temp struct
    for (unsigned int i=0; i<sizeof(configuration_type); i++){
      *((char*)&CONFTMP + i) = EEPROM.read(CONFIG_START + i);
    }
  }
  if (temp != runNumber ||
      strcmp(CONFTMP.ssid, ssid) != 0 ||
      strcmp(CONFTMP.pass, pass) != 0 ||
      strcmp(CONFTMP.mqtt_server, mqtt_server) != 0 ||
      strcmp(CONFTMP.mqtt_user, mqtt_user) != 0 ||
      strcmp(CONFTMP.mqtt_password, mqtt_password) != 0 ||
      CONFTMP.mqtt_port != mqtt_port ||
      CONFTMP.coConstTempCutOff != coConstTempCutOff ||
      CONFTMP.forceCObelow != forceCObelow  ||
      CONFTMP.histereza != histereza ||
      CONFTMP.forceWater != forceWater ||
      CONFTMP.forceCO != forceCO) {  //skip save if runnumber = saved runnumber to avoid too much memory save and wear eeprom
    EEPROM.put(1, runNumber);
    //EEPROM.put(1+sizeof(runNumber), flame_used_power_kwh);
    log_message((char*)F("Saving config........................... to EEPROM some data changed"));

    strcpy(CONFIGURATION.version,CONFIG_VERSION);
    strcpy(CONFIGURATION.ssid,ssid);
    strcpy(CONFIGURATION.pass,pass);
    strcpy(CONFIGURATION.mqtt_server,mqtt_server);
    strcpy(CONFIGURATION.mqtt_user,mqtt_user);
    strcpy(CONFIGURATION.mqtt_password,mqtt_password);
    CONFIGURATION.mqtt_port = mqtt_port;
    CONFIGURATION.najpierwCO = najpierwCO;
    CONFIGURATION.coConstTempCutOff = coConstTempCutOff;
    CONFIGURATION.forceCObelow = forceCObelow;
    CONFIGURATION.histereza = histereza;
    CONFIGURATION.forceWater = forceWater;
    CONFIGURATION.forceCO = forceCO;
    for (unsigned int i=0; i<sizeof(configuration_type); i++)
      { EEPROM.write(CONFIG_START + i, *((char*)&CONFIGURATION + i)); }
    EEPROM.commit();
  }
}

void SaveEnergy() {
   // ------------------------ energy config save --------------
  unsigned int temp = 0;
  EEPROM.get(1,temp);
  if (temp != runNumber ) {EEPROM.put(1, runNumber);}
  double dtemp =0;
  EEPROM.get(1+sizeof(runNumber),dtemp);
  if (dtemp != energy1used) {EEPROM.put(1+sizeof(runNumber), energy1used);}
  EEPROM.get(1+sizeof(runNumber)+sizeof(energy1used),dtemp);
  if (dtemp != energy2used) {EEPROM.put(1+sizeof(runNumber)+sizeof(energy1used), energy2used);}
  EEPROM.commit();
  log_message((char*)F("saved energy used... only if needed"));

}

//
//bool isPnpDeviceAvailable( uint8_t iAddress )
//{
//  Wire.beginTransmission((int)iAddress);
//  bool result= Wire.write( (int)0x00 ) == 0x00;
//  Wire.endTransmission();
//  return result;
//}

double getenergy(int adspin) {
  //  double kilos = 0;
  //  int peakPower = 0;
  //  int16_t  adc0, adc1, adc2, adc3, adc01, adc23;
  //  double adf0, adf1, adf2, adf3;
  int current = 0;
  int maxCurrent = 0;
  int minCurrent = 65536 / 2;
  double measureTime = millis();
  //long czaskonwersji = millis();
  #ifdef debug
    Serial.print("adspin: ");
    Serial.println(adspin);
  #endif
  if (isadslinitialised) {
    for (int i = 0 ; i <= 75 ; i++) //Monitors and logs the current input for 200 cycles to determine max and min current
    {
      #ifdef wdtreset
      wdt_reset();
      #endif
      if (adspin == 0)     current = ads1.readADC_Differential_0_1();  //Reads current input and records maximum and minimum current
      if (adspin == 1)     current = ads1.readADC_Differential_2_3();
      if (current >= maxCurrent)
        maxCurrent = current;
      else if (current <= minCurrent)
        minCurrent = current;
    }
  } else {
    current = 0;
    #ifdef debug
      Serial.println(F("No I2C device available -force current=0"));
    #endif
  }
  if (maxCurrent>13333) maxCurrent = maxCurrent-13333;
  double RMSCurrent = ((maxCurrent)*multiplier/118337)*10; //current = ((adc0 - 13333) * 0.1875 / sensitivity); //((maxCurrent - 11826)*0.00707)/multiplier; //(multiplier*maxCurrent)/100*(5*1,414/1000) *1000; //((maxCurrent ) * multiplier) / 10000 * 2 * 0.88; //11.8337; //(multiplier*maxCurrent)/100*(5*1,414/1000) *1000;  //((maxCurrent - 11826)*0.00707)/multiplier;    //Calculates RMS current based on maximum value
  if (RMSCurrent < 0.0001) RMSCurrent = 0;
#ifdef debug
  Serial.print(adspin);
  Serial.print(F(" : "));
  Serial.print(RMSCurrent * 1000, 4);
  Serial.print(F("mA  Czas konwersji: "));
  Serial.print((millis())/1);
  Serial.print(F("ms  multiplier: "));
  Serial.print(multiplier);
  Serial.print(F("  MaxCurrent: "));
  Serial.println(maxCurrent,4);
#endif
  //    if (RMSCurrent > 20 and (prgstatusrelay1WO != digitalRead(relay1) or prgstatusrelay2CO != digitalRead(relay2))) {
  //  //    //wylacz bezposrednio dla upewnienia sie czy przelacznik zewnetrzny force jest wylaczony i sterowanie ma kontrolowac
  //      Serial.print("Forced pump by button!!!!!!!!!!!: ");
  //      Serial.println(adspin);
  //      if (adspin == 0) SuplaDevice.relayOn(keyChannelNumberRelay1, 0);
  //      if (adspin == 1) SuplaDevice.relayOn(keyChannelNumberRelay2, 0);
  //    } else {
  //      Serial.print("Forced off button");
  //      if (adspin == 0)  SuplaDevice.relayOff(keyChannelNumberRelay1);
  //      if (adspin == 1)  SuplaDevice.relayOff(keyChannelNumberRelay2);
  //    }
  //return Ah
  const double hour = 60 * 60 * 1000;
  if (RMSCurrent < (0.5/commonVolt)) RMSCurrent = 0;  //ogranicz minimum do 0,5W
  return ((millis()-measureTime)/hour) * RMSCurrent*0.707 * ((millis()-measureTime)/ReadEnergyTimeInterval); //return real current nmqttident max rms
}



#ifdef enableWebSerial
void recvMsg(uint8_t *data, size_t len)
{ // for WebSerial
  String d = "";
  for (size_t i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  d.toUpperCase();
  sprintf(log_chars,"Received Data on WebSerial...: %s", String(d).c_str());
  log_message(log_chars);
  if (d == "ON")
  {
    //  digitalWrite(LED, HIGH);
  } else
  if (d == "OFF")
  {
    //  digitalWrite(LED, LOW);
  } else
  if (d == "RESTART")
  {
    WebSerial.print(String(millis())+": ");
    WebSerial.println(F("OK. Restarting... by command..."));
    restart();
  } else
  if (d == "RECONNECT")
  {
    mqtt_reconnect();
  } else
  if (d == "SAVE")
  {
    WebSerial.print(String(millis())+": ");
    WebSerial.println(F("Saving config to EEPROM memory by command..."));
    WebSerial.println("Size CONFIG: " + String(sizeof(CONFIGURATION)));
    SaveConfig();
  } else
  if (d == "RESET_CONFIG")
  {
    WebSerial.print(String(millis())+": ");
    WebSerial.println(F("RESET config to DEFAULT VALUES and restart..."));
    WebSerial.println("Size CONFIG: " + String(sizeof(CONFIGURATION)));
    CONFIGURATION.version[0] = 'R';
    CONFIGURATION.version[1] = 'E';
    CONFIGURATION.version[2] = 'S';
    CONFIGURATION.version[3] = 'E';
    CONFIGURATION.version[4] = 'T';
    SaveConfig();
    restart();
  } else
  if (d.indexOf("FORCECOBELOW") !=- 1)
  {
    String part = d.substring(d.indexOf(" "));
    part.trim();
    WebSerial.print(String(millis())+F(": ForceCOBelow: ")+String(forceCObelow)+F("   "));
    if (d.indexOf(" ")!=-1) {
      if (PayloadtoValidFloatCheck(part)) {forceCObelow = PayloadtoValidFloat(part,true,4,15);}
      WebSerial.println(F(" -> ZMIENIONO NA: ")+String(forceCObelow)+F("    Payload: ")+String(d));
    } else { WebSerial.println("");}
  } else
  if (d.indexOf("COCUTOFFTEMP") !=- 1)
  {
    String part = d.substring(d.indexOf(" "));
    part.trim();
    WebSerial.print(String(millis())+F(": COCUTOFFTEMP: ")+String(coConstTempCutOff)+F("   "));
    if (d.indexOf(" ")!=-1) {
      if (PayloadtoValidFloatCheck(part)) {coConstTempCutOff = PayloadtoValidFloat(part,true,20,40);}
      WebSerial.println(F(" -> ZMIENIONO NA: ")+String(coConstTempCutOff)+F("    Payload: ")+String(d));
    } else { WebSerial.println("");}
  } else
  if (d.indexOf("HIST") !=- 1)
  {
    String part = d.substring(d.indexOf(" "));
    part.trim();
    WebSerial.print(String(millis())+F(": HISTEREZA: ")+String(histereza)+F("   "));
    if (d.indexOf(" ")!=-1) {
      if (PayloadtoValidFloatCheck(part)) {histereza = PayloadtoValidFloat(part,true,-5,5);}
      WebSerial.println(F(" -> ZMIENIONO NA: ")+String(histereza)+F("    Payload: ")+String(d));
    } else {WebSerial.println("");}
  } else
  if (d.indexOf("FORCECO") !=- 1)
  {
    String part = d.substring(d.indexOf(" "));
    part.trim();
    WebSerial.print(String(millis())+F(": ForceCO: ")+String(forceCO?"ON":"OFF"));
    if (d.indexOf(" ")!=-1) {
      if (PayloadStatus(part,true)) {forceCO = true;} else if (PayloadStatus(part,false)) {forceCO = false;}
      WebSerial.println(F(" -> ZMIENIONO NA: ")+String(forceCO?"ON":"OFF")+F("    Payload: ")+String(d));
    } else {WebSerial.println("");}
  } else
  if (d.indexOf("FORCEWATER") !=- 1)
  {
    String part = d.substring(d.indexOf(" "));
    part.trim();
    WebSerial.print(String(millis())+F(": ForceWater: ")+String(forceWater?"ON":"OFF"));
    if (d.indexOf(" ")!=-1) {
      if (PayloadStatus(part,true)) {forceWater = true;} else if (PayloadStatus(part,false)) {forceWater = false;}
      WebSerial.println(F(" -> ZMIENIONO NA: ")+String(forceWater?"ON":"OFF")+F("    Payload: ")+String(d));
    } else {WebSerial.println("");}
  } else
  if (d.indexOf("LOG2MQTT") !=- 1)
  {
    String part = d.substring(d.indexOf(" "));
    part.trim();
    WebSerial.print(String(millis())+F(": LOG2MQTT: ")+String(sendlogtomqtt?"ON":"OFF"));
    if (d.indexOf(" ")!=-1) {
      if (PayloadStatus(part,true)) {sendlogtomqtt = true;} else if (PayloadStatus(part,false)) {sendlogtomqtt = false;}
      WebSerial.println(F(" -> ZMIENIONO NA: ")+String(sendlogtomqtt?"ON":"OFF")+F("    Payload: ")+String(d));
    } else {WebSerial.println("");}
  }else
  if (d == "HELP")
  {
    log_message((char*)F("KOMENDY:\n \
      FORCECOBELOW xx  -Zmienia wartość xx 'wymusza pompe CO poniżej temperatury średniej zewnetrznej', \n \
      COCUTOFFTEMP xx  -Zmienia wartość xx 'Temperatura graniczna na wymienniku oznacza ze piec sie grzeje',\n \
      HIST xx          -Zmienia wartość xx histerezy progu grzania,\n \
      ForceCO 0/1      -1(ON) -Wymusza grzanie CO 0(OFF) -deaktywuje,\n \
      ForceWater 0/1   -1(ON) -Wymusza grzanie Wody 0(OFF) -deaktywuje,\n \
      LOG2MQTT         -1(ON) wysyla log do MQTT 0 wylacza,\n \
      RESTART          -Uruchamia ponownie układ,\n \
      RECONNECT        -Dokonuje ponownej próby połączenia z bazami,\n \
      SAVE             -Wymusza zapis konfiguracji,\n \
      RESET_CONFIG     -UWAGA!!!! Resetuje konfigurację do wartości domyślnych"));
  }


}
#endif

#ifdef enableMQTT
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  #include "configmqtttopics.h"
  const String topicStr(topic);

  String payloadStr = convertPayloadToStr(payload, length);

//get Voltage on same phase
  if (topicStr == SUPLA_VOLT_TOPIC)
  {
    String ident = String(millis())+F(": electricmain Volt ");
    if (PayloadtoValidFloatCheck(payloadStr))
    {
      commonVolt = PayloadtoValidFloat(payloadStr,true);     //true to get output to serial and webserial atof(paylstr.c_str());
      // char result[6];
      // dtostrf(commonVolt, 4, 2, result);
//      receivedmqttdata = true;    //makes every second run mqtt send and influx
    }
  } else
//GET FREQ from supla
  if (topicStr == SUPLA_FREQ_TOPIC)
  {
    String ident = String(millis())+F(": electricmain Frequency ");
    if (PayloadtoValidFloatCheck(payloadStr))
    {
      commonFreq = PayloadtoValidFloat(payloadStr,true);     //true to get output to serial and webserial atof(paylstr.c_str());
        // commonFreq = atof(paylstr.c_str());
        // char result[6];
        // dtostrf(commonFreq, 4, 2, result);
    }
  } else
//PUMP1
  if (topicStr == BOILERROOM_SWITCH_TOPIC_SET+"_"+BOILERROOM_PUMP1WA)
  {
    String ident = String(F("Boiler PUMP1 Wather. "));
    payloadStr.toUpperCase();
    ident += F("Set mode: ");
    bool tmp = PayloadStatus(payloadStr, true) && !PayloadStatus(payloadStr, false);
    if (tmp != prgstatusrelay1WO) receivedmqttdata = true;
    if (PayloadStatus(payloadStr, true))
    {
      ownother=F("<B> MQTT pump1WO ")+payloadStr+F("</B>");
      prgstatusrelay1WO = true;
      forceWater = true;
      forceCO = false;
      najpierwCO = false;
      ident += "WO mode " + payloadStr;
    }
    else if (PayloadStatus(payloadStr, false))
    {
      prgstatusrelay1WO = false;
      forceWater = false;
      forceCO = false;
      najpierwCO = false;
      ident +="WO mode " + payloadStr;
    }
    else {
      ident += F(" unknkown ");
    }
    log_message((char*)ident.c_str());
  } else
//PUMP2
  if (topicStr == BOILERROOM_SWITCH_TOPIC_SET+"_"+BOILERROOM_PUMP2CO)
  {
    String ident = F("Boiler PUMP2 CO. ");
    payloadStr.toUpperCase();
    ident += F("Set mode: ");
    bool tmp = PayloadStatus(payloadStr, true) && !PayloadStatus(payloadStr, false);
    if (tmp != prgstatusrelay2CO) receivedmqttdata = true;
    if (PayloadStatus(payloadStr, true))
    {
      ownother=F("<B> MQTT pump2CO ")+payloadStr+F("</B>");
      prgstatusrelay2CO = true;
      forceCO = true;
      waitCOStartingmargin=millis();
      forceWater = false;
      najpierwCO = true;
      ident +="CO mode " + payloadStr;
    }
    else if (PayloadStatus(payloadStr, false))
    {
      prgstatusrelay2CO = false;
      forceCO = false;
      forceWater = false;
      najpierwCO = false;
      ident += "CO mode " + payloadStr;
    }
    else {
      ident += F(" unknkown ");
    }
    log_message((char*)ident.c_str());
  }
   receivedmqttdata = false;
}
#endif

void mqtt_reconnect_subscribe_list()
{
  #ifdef enableMQTT
      mqttclient.subscribe(SUPLA_VOLT_TOPIC.c_str());
      mqttclient.subscribe(SUPLA_FREQ_TOPIC.c_str());
      mqttclient.subscribe((BOILERROOM_SWITCH_TOPIC_SET+"_"+BOILERROOM_PUMP1WA).c_str());
      mqttclient.subscribe((BOILERROOM_SWITCH_TOPIC_SET+"_"+BOILERROOM_PUMP2CO).c_str());
  #endif
}

#ifdef ENABLE_INFLUX
void updateInfluxDB()
{
  #include "configmqtttopics.h"
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
  InfluxSensor.addField(mqttident + "rssi", (WiFi.RSSI()));
  InfluxSensor.addField(mqttident + "CRT",  (runNumber));
  InfluxSensor.addField(mqttident + "uptime",  ((millis())/1000));   //w sekundach

  if (bmTemp != InitTemp) {InfluxSensor.addField(mqttident + String(BOILERROOM_TEMPERATURE), bmTemp);}
  InfluxSensor.addField(mqttident + String(BOILERROOM_PRESSURE), dbmpressval);
  InfluxSensor.addField(mqttident + String(BOILERROOM_HIGH), bm_high);
  InfluxSensor.addField(mqttident + String(BOILERROOM_HIGHREAL), bm_high_real);
  InfluxSensor.addField(mqttident + String(BOILERROOM_COVAL), dcoval);
  if (coTherm != InitTemp) {InfluxSensor.addField(mqttident + String(HEATERCO_TEMPERATURE), coTherm);}
  if (waterTherm != InitTemp) {InfluxSensor.addField(mqttident + String(WATER_TEMPERATURE), waterTherm);}
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP1WA), prgstatusrelay1WO?"1":"0");
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP1WA_E), energy1used);
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP2CO_E), energy2used);
    InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP2CO), prgstatusrelay2CO?"1":"0");
  if (NTherm != InitTemp) {InfluxSensor.addField(mqttident + String(OUTSIDE_TEMPERATURE_N), NTherm);}
  if (ETherm != InitTemp) {InfluxSensor.addField(mqttident + String(OUTSIDE_TEMPERATURE_E), ETherm);}
  if (WTherm != InitTemp) {InfluxSensor.addField(mqttident + String(OUTSIDE_TEMPERATURE_W), WTherm);}
  if (STherm != InitTemp) {InfluxSensor.addField(mqttident + String(OUTSIDE_TEMPERATURE_S), STherm);}
  if (OutsideTempAvg != InitTemp) {InfluxSensor.addField(mqttident + String(OUTSIDE_TEMPERATURE_A), OutsideTempAvg);}

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

void mqttHAPublish_Config (String HADiscoveryTopic, String ValueTopicName, String SensorName, String friendlySensorName, int unitClass, String cmd_temp){
  #ifdef enableMQTT
  #include "configmqtttopics.h"
  String mqttdeviceid = String(BASE_TOPIC);
  String HAClass;
  switch (unitClass) {
    case mqtt_HAClass_temperature:   HAClass = F("\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\","); break;
    case mqtt_HAClass_pressure:      HAClass = F("\"dev_cla\":\"pressure\",\"unit_of_meas\": \"hPa\",\"ic\": \"mdi:mdiCarSpeedLimiter\",");  break; //cbar, bar, hPa, inHg, kPa, mbar, Pa, psi
    case mqtt_HAClass_humidity:      HAClass = F("\"dev_cla\":\"humidity\",\"unit_of_meas\": \"%\",\"ic\": \"mdi:mdiWavesArrowUp\","); break;
    case mqtt_HAClass_energy:        HAClass = F("\"dev_cla\":\"energy\",\"unit_of_meas\": \"kWh\",\"state_class\":\"total_increasing\",\"ic\": \"mdi:lightning-bolt-circle\","); break; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
//    case mqtt_HAClass_power:         HAClass = F("\"dev_cla\":\"power\",\"unit_of_meas\": \"W\",\"ic\": \"mdi:alpha-W-box\","); break; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
//    case mqtt_HAClass_voltage:       HAClass = F("\"dev_cla\":\"voltage\",\"unit_of_meas\": \"V\",\"ic\": \"mdi:alpha-v-box\","); break; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
//    case mqtt_HAClass_current:       HAClass = F("\"dev_cla\":\"current\",\"unit_of_meas\": \"A\",\"ic\": \"mdi:alpha-a-box\","); break; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
//    case mqtt_HAClass_freq:          HAClass = F("\"dev_cla\":\"frequency\",\"unit_of_meas\": \"Hz\",\"ic\": \"mdi:sine-wave\","); break; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
    case mqtt_HAClass_high:          HAClass = F("\"unit_of_meas\": \"m\",\"ic\": \"mdi:speedometer-medium\","); break;
    case mqtt_HAClass_co:            HAClass = F("\"dev_cla\":\"carbon_monoxide\",\"unit_of_meas\": \"ppm\",\"ic\": \"mdi:molecule-co\","); break;
    case mqtt_HAClass_switch:        HAClass = F("\"pl_off\":\"OFF\",\"pl_on\":\"ON\",");
                                     if (cmd_temp.length()>0) {HAClass +=F("\"cmd_t\":\"") + cmd_temp + F("_") + SensorName + F("\",");} break;
    default:                         HAClass = "\0"; break;
  }
  mqttclient.publish((HADiscoveryTopic + F("_") + mqttident + SensorName + F("/config")).c_str(), (F("{\"name\":\"") + mqttident + friendlySensorName + F("\",\"uniq_id\": \"") + mqttident + SensorName + F("\",\"stat_t\":\"") + ValueTopicName + F("\",\"val_tpl\":\"{{value_json.") + mqttident + SensorName +F("}}\",") + HAClass + F("\"qos\":") + mqttQOS + F(",") + mqttdeviceid + F("}")).c_str(), mqtt_Retain);
  #endif
}


void updateMQTTData() {
  #ifdef enableMQTT
  #include "configmqtttopics.h"
  String mqttdeviceid = String(BASE_TOPIC);
  const String payloadvalue_startend_val = F("\0"); // value added before and after value send to mqtt queue
  String tmpbuilder = F("{");
  tmpbuilder += F("\"rssi\":")+ String(WiFi.RSSI());
  tmpbuilder += F(",\"CRT\":")+ String(runNumber);
  tmpbuilder += F(",\"uptime\":")+ String((millis())/1000);   //w sekundach
  if (NTherm != InitTemp) {tmpbuilder += F(",\"") + mqttident + OUTSIDE_TEMPERATURE_N + F("\": ") + payloadvalue_startend_val + String(NTherm) + payloadvalue_startend_val;}
  if (ETherm != InitTemp) {tmpbuilder += F(",\"") + mqttident + OUTSIDE_TEMPERATURE_E + F("\": ") + payloadvalue_startend_val + String(ETherm) + payloadvalue_startend_val;}
  if (WTherm != InitTemp) {tmpbuilder += F(",\"") + mqttident + OUTSIDE_TEMPERATURE_W + F("\": ") + payloadvalue_startend_val + String(WTherm) + payloadvalue_startend_val;}
  if (STherm != InitTemp) {tmpbuilder += F(",\"") + mqttident + OUTSIDE_TEMPERATURE_S + F("\": ") + payloadvalue_startend_val + String(STherm) + payloadvalue_startend_val;}
  if (OutsideTempAvg != InitTemp) {tmpbuilder += F(",\"") + mqttident + OUTSIDE_TEMPERATURE_A + F("\": ") + payloadvalue_startend_val + String(OutsideTempAvg) + payloadvalue_startend_val;}
  if (coTherm != InitTemp) {tmpbuilder += F(",\"") + mqttident + HEATERCO_TEMPERATURE + F("\": ") + payloadvalue_startend_val + String(coTherm) + payloadvalue_startend_val;}
  if (waterTherm != InitTemp) {tmpbuilder += F(",\"") + mqttident + WATER_TEMPERATURE + F("\": ") + payloadvalue_startend_val + String(waterTherm) + payloadvalue_startend_val;}
  if (bmTemp != InitTemp) {tmpbuilder += F(",\"") + mqttident + BOILERROOM_TEMPERATURE + F("\": ") + payloadvalue_startend_val + String(bmTemp) + payloadvalue_startend_val;}
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PRESSURE + F("\": ") + payloadvalue_startend_val + String(dbmpressval) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_HIGH + F("\": ") + payloadvalue_startend_val + String(bm_high) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_HIGHREAL + F("\": ") + payloadvalue_startend_val + String(bm_high_real) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_COVAL + F("\": ") + payloadvalue_startend_val + String(dcoval) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP1WA_E + F("\": ") + payloadvalue_startend_val + String(energy1used) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP2CO_E + F("\": ") + payloadvalue_startend_val + String(energy2used) + payloadvalue_startend_val;
  //pump status
  mqttclient.publish(BOILERROOM_SENSOR_TOPIC.c_str(),(tmpbuilder+F("}")).c_str(), mqtt_Retain);

  tmpbuilder = F("{");
  tmpbuilder += F("\"CRT\":")+ String(runNumber);
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP1WA + F("\": ") + payloadvalue_startend_val + String(prgstatusrelay1WO?"\"ON\"":"\"OFF\"") + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP2CO + F("\": ") + payloadvalue_startend_val + String(prgstatusrelay2CO?"\"ON\"":"\"OFF\"") + payloadvalue_startend_val;   //getpumpstatus(2)
  mqttclient.publish(BOILERROOM_SWITCH_TOPIC.c_str(),(tmpbuilder+F("}")).c_str(), mqtt_Retain);

  publishhomeassistantconfig++; // zwiekszamy licznik wykonan wyslania mqtt by co publishhomeassistantconfigdivider wysłań wysłać autoconfig discovery dla homeassisatnt
  if (publishhomeassistantconfig % publishhomeassistantconfigdivider == 0)
  {
    //Make Homeassistant autodiscovery and autoconfig
    //Temperatures
    if (NTherm!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_N, dNThermometerS, mqtt_HAClass_temperature);
    if (ETherm!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_E, dEThermometerS, mqtt_HAClass_temperature);
    if (WTherm!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_W, dWThermometerS, mqtt_HAClass_temperature);
    if (STherm!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_S, dSThermometerS, mqtt_HAClass_temperature);
    if (OutsideTempAvg!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_A, dallThermometerS, mqtt_HAClass_temperature);
    if (coTherm!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, HEATERCO_TEMPERATURE, dcoThermstat, mqtt_HAClass_temperature);
    if (waterTherm!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, WATER_TEMPERATURE, dwaterThermstat, mqtt_HAClass_temperature);
    if (bmTemp!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_TEMPERATURE, dbmtemperature, mqtt_HAClass_temperature);
    //pressure
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PRESSURE, dbmpressure, mqtt_HAClass_pressure);
    //high
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_HIGH, dbmhigh, mqtt_HAClass_high);
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_HIGHREAL, dbmhighr, mqtt_HAClass_high);
    //CO ppm
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_COVAL, dcoS, mqtt_HAClass_co);
    //pumps energy
    // mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP1WA_E, dpump1energyS, mqtt_HAClass_energy);
    // mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP2CO_E, dpump2energyS, mqtt_HAClass_energy);

    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP1WA_E, dpump1energyS, mqtt_HAClass_energy);
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP2CO_E, dpump2energyS, mqtt_HAClass_energy);
    //pumps switch/state
    mqttHAPublish_Config(BOILERROOM_HA_SWITCH_TOPIC, BOILERROOM_SWITCH_TOPIC, BOILERROOM_PUMP1WA, dpump1, mqtt_HAClass_switch, BOILERROOM_SWITCH_TOPIC_SET);
    mqttHAPublish_Config(BOILERROOM_HA_SWITCH_TOPIC, BOILERROOM_SWITCH_TOPIC, BOILERROOM_PUMP2CO, dpump2, mqtt_HAClass_switch, BOILERROOM_SWITCH_TOPIC_SET);
  }

  // }
  log_message((char*)F("MQTT Data Sended..."));
  #endif
}
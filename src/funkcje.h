
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
  //byte tempDevices[sensor_18b20_numer][8];     // Up to five temperature sensors
  int tempDeviceCount;        // temperature sensor count
  uint8_t nowe18b = 0;      //ilosc nowych czujnikow
  OneWire ow(ONE_WIRE_BUS);
  DallasTemperature sensors(&ow);
  sensors.begin(); // initialize sensors
  tempDeviceCount = sensors.getDeviceCount();
#ifdef debug
  Serial.print("Device Count: ");
  Serial.print(tempDeviceCount);
  Serial.println();
#endif
  String nowe18baddr = "<p>New unassigned:<br>";
  uint8_t unassigned = 0;
  for (int i = 0; i < tempDeviceCount; i++) {// Create w1 device index array
    byte addr[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    if (sensors.getAddress(addr, i)) {
      for (uint8_t x = 0; x < sensor_18b20_numer; x++) {
        uint8_t count = 0;
        for (uint8_t x1 = 0; x1 < 8; x1++) {
          if (addr[x1] == sts[x].ssn[x1]) {
            count++;
          }
          if (count == 8) addr[0] = 0; //wyzeruj pierwszy bo dopasowany
        }
      }
      if (addr[0] != 0) {
        //nowy numer
        char tmpsn[16];
        array_to_string(addr, 8, tmpsn);
        nowe18b++;
        nowe18baddr += tmpsn;
        nowe18baddr += " : ";
        sensors.requestTemperaturesByAddress(addr);
        unassigned++;
        nowe18baddr += String(sensors.getTempC(addr)); //getTempC(addr);
        nowe18baddr += "&deg;C";
        nowe18baddr += "<br>";
      }
    }
  }
  nowe18baddr += "<br></p>";
  if (unassigned == 0) nowe18baddr = "\0";
  return nowe18baddr;
}



//******************************************************************************************

String PrintHex8(const uint8_t *data, char separator, uint8_t length) // prints 8-bit data in hex , uint8_t length
{
  uint8_t lensep = sizeof(separator);
  int dod = 0;
  if (separator == 0x0) {
    lensep = 0;
    dod = 1;
  }
  wdt_reset();
  if (lensep > 1) dod = 1 - lensep;
  char tmp[length * (2 + lensep) + dod - lensep];
  byte first;
  byte second;
  for (int i = 0; (i + 0) < length; i++) {
    first = (data[i] >> 4) & 0x0f;
    second = data[i] & 0x0f;
    // base for converting single digit numbers to ASCII is 48
    // base for 10-16 to become lower-case characters a-f is 87
    // nmqttidente: difference is 39
    tmp[i * (2 + lensep)] = first + 48;
    tmp[i * (2 + lensep) + 1] = second + 48;
    if ((i) < length and (i) + 1 != length) tmp[i * (2 + lensep) + 2] = separator;
    if (first > 9) tmp[i * (2 + lensep)] += 39;
    if (second > 9) tmp[i * (2 + lensep) + 1] += 39;

  }
  tmp[length * (2 + lensep) + 0 - lensep] = 0;
#ifdef debug
  Serial.print(F("MAC Addr: "));
  Serial.println(tmp);
#endif
  //     debugA("%s",tmp);
  return tmp;
}

//*********************************************************************************************

void WiFi_up() { // conect to wifi
  if (millis() > wifimilis)  {
    webserver.end();
    WiFi.disconnect();
    delay(500);
    WiFi.begin(WSSID, WPass);
    Serial.println("CONNECTING WIFI");
    starting = true;
  }
  wifimilis = (millis() + wifi_checkDelay) ;
}
//******************************************************************************************
//     background-color: #01DF3A;
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
//******************************************************************************************
String getpumpstatus(uint8_t pompa) {

  if (pompa == 1) {
    String ptr;
    if (!prgstatusrelay1WO or pump1energyS->getlast(energy_current).toDouble() < pumpmincurrent) {  //digitalRead(relay1) == HIGH
      ptr = F("<B>Stoi</B> ");
      ptr += pump1energyS->getlast(energy_power);
      ptr += F("W</B><BR>");
      if (najpierwCO) {
        ptr += F("<font color=\"red\"><sup class='units'>Zmiana priorytetu na CO: ");
        ptr += String(najpierwCO);
        ptr += F("</sup></font>");
      }
    } else {
      ptr = F("<B><font color=\"red\">Chodzi</font> ");
      ptr += pump1energyS->getlast(energy_power);
      ptr += F("W</B><BR>");
      if (najpierwCO) {
        ptr += F("<font color=\"red\"><sup class='units'>Zmiana priorytetu na CO: ");
        ptr += String(najpierwCO);
        ptr += F("</sup></font>");
      }
    } return String(ptr).c_str();
  } else {
    String ptr;
    if (!prgstatusrelay2CO or pump2energyS->getlast(energy_current).toDouble() < pumpmincurrent) {  //digitalRead(relay2) == HIGH
      ptr = F("<B>Stoi</B> ");
      ptr += pump2energyS->getlast(energy_power);
      ptr += F("W</B><BR>");
      ptr += F("<sup class='units'>histereza: ");
      ptr += String(histereza);
      ptr += F("</sup>");
    } else {
      ptr = F("<B><font color=\"red\">Chodzi</font> ");
      ptr += pump2energyS->getlast(energy_power);
      ptr += F("W</B><BR> <sup class='units'>histereza: ");
      ptr += String(histereza);
      ptr += F("</sup>");
    } return String(ptr);
  }
  return "";
}
// Replaces placeholder with DHT values
String processor(const String var) {

  wdt_reset();
  #ifdef debug
//    Serial.print(F("Start processor: "));
//    Serial.println(var);
  #endif
  if (var == "ver") {
    String a = "</B>ESP CO Server dla: <B>" + String(me_lokalizacja) + "</B><BR>v. ";
    a += me_version;
    a += "<br><font size=\"2\" color=\"DarkGreen\">";
    a += espClient.connected()? "MQTT "+String(msg_Connected)+": "+String(mqtt_server)+":"+String(mqtt_port) : "MQTT "+String(msg_disConnected)+": "+String(mqtt_server)+":"+String(mqtt_port) ;  //1 conn, 0 not conn
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
    return ptr.c_str();
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
    return ptr.c_str();
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
    #ifdef newSensorT1
    tmp=String(dT1ThermometerS); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    #endif
    tmp=String(dallThermometerS); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;
    tmp=String(do_stopkawebsiteS); refreshtime+=step; ptr+=function0+ tmp +function1+ tmp +function2+ String(refreshtime) +function3;

    //ptr+="if(document.getElementById(\"blink\")){var blink=document.getElementById('blink');setInterval(function(){blink.style.color=(blink.style.color=='red'?'blue':'red');}, 1200);}"; //blink
    ptr+=F("function bt(){document.querySelectorAll('.blink').forEach(e =>{setInterval(()=>{console.log(e);e.style.color=(blink.style.color=='red'?'blue':'red')},500);});}document.addEventListener('DOMContentLoaded',()=>{bt();});"); //classList.toggle('hide');
    Serial.println(F("scriptsectionreplace: ")+String(ptr.length()));
    return String(ptr).c_str();;
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
    a += F("kWh</b><br> ads initialised: <b>");
    a += String(isadslinitialised ? "Tak" : "Nie");
    a += F("</B>");
    if (String(checkUnassignedSensors()).length()>0) a += F("<br> Unassigned: ") + String(checkUnassignedSensors());
    if (ownother.length()>0) { a += F("<br> Własny string: ")+ String(ownother); }
    return String(a);
  }
  if (var == "uptime") {
    return String(uptimelink).c_str();;
  }
  if (var == "uptimewart") {
    return String(uptimedana(0)).c_str(); //0 bo czas startu od milis
  }
  if (var == "me_lokalizacja") {
    return String(me_lokalizacja).c_str();;
  }
  Serial.println(String(millis())+": "+String(var));
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

    return String(ptr).c_str();;
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
    return String(ptr).c_str();
  }
  if (var == "bodywstaw") {
    String ptr =F("\0");
    ptr=F("<form action=\"/get\"><table>");
//    ptr+="<table>";
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_NEWS)+F("</span><B><span class=\"dht-labels-temp\" id=\"")+String(dallThermometerS)+F("\">&nbsp;<font color=\"Green\">")+String(OutsideTempAvg)+F("</font></span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("<tr><td>");
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_COHeat)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dcoThermstat)+F("\">&nbsp;<font color=\"Blue\">")+String(coTherm,1)+F("</font></span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("<td></td");
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_WaterCOHeat)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dwaterThermstat)+F("\">&nbsp;<font color=\"blue\">")+String(waterTherm,1)+F("</font></span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td></tr>");
    ptr+=F("<tr><td>");
    ptr+="<p>"+ppmicon+F("<span class=\"dht-labels\">")+String(pump)+F(" 1 ")+String(CO_heat)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dpump1)+F("\">&nbsp;")+String(getpumpstatus(1))+F("</span><sup class=\"units\"> </sup></B></p>");
    ptr+=F("<td></td");
    ptr+="<p>"+ppmicon+F("<span class=\"dht-labels\">")+String(pump)+F(" 2 ")+String(water)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dpump2)+F("\">&nbsp;")+String(getpumpstatus(2))+F("</span><sup class=\"units\"> </sup></B></p>");
    ptr+=F("</td></tr>");


    return ptr.c_str();
  }
if (var == "bodywstaw1") {
    String ptr ="\0";
    //if (NThermometerS->getlast()==InitTemp) wart="--.--"; else wart=NThermometerS->getlast();
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
    //if (NThermometerS->getlast()<-100 or NThermometerS->getlast()>100) wart="-.--"; else wart=String(NThermometerS->getlast());
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_N)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dNThermometerS)+F("\">&nbsp;")+String(NThermometerS->getlast(),1)+F("</span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td><td>");
    //if (EThermometerS->getlast()<-100 or EThermometerS->getlast()>100) wart="-.--"; else wart=String(EThermometerS->getlast());
    ptr+="<p>"+tempicon+F("<span class=\"dht-labels\">")+String(Temp_E)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dEThermometerS)+F("\">&nbsp;")+String(EThermometerS->getlast(),1)+F("</span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td></tr><tr><td>");
    return ptr.c_str();
  }
if (var == "bodywstaw2") {
    String ptr =F("\0");
    //if (WThermometerS->getlast()<-100 or WThermometerS->getlast()>100) wart="-.--"; else wart=String(WThermometerS->getlast());
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_W)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dWThermometerS)+F("\">&nbsp;")+String(WThermometerS->getlast(),1)+F("</span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td><td>");
    //if (SThermometerS->getlast()<-100 or SThermometerS->getlast()>100) wart="-.--"; else wart=String(EThermometerS->getlast());
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_S)+F("</span><BR><B><span class=\"dht-labels-temp\" id=\"")+String(dSThermometerS)+F("\">&nbsp;")+String(SThermometerS->getlast(),1)+F("</span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td></tr>");
    ptr+=F("<tr><td>");
    ptr+=F("<p>")+tempicon+F("<span class=\"dht-labels\">")+String(Temp_BoilerRoom)+F("</span><B><span class=\"dht-labels-temp\" id=\"")+String(dbmtemperature)+F("\">&nbsp;")+String(bmTemp,1)+F("</span><sup class=\"units\">&deg;C</sup></B></p>");
    ptr+=F("</td></tr>");
    #ifdef newSensorT1
    ptr+="<p>"+tempicon+F("<span class=\"dht-labels\">")+String(Temp_T1)+"</span><B><span class=\"dht-labels-temp\" id=\""+String(dT1ThermometerS)+"\"&nbsp;>"+String(T1ThermometerS->getlast())+"</span><sup class=\"units\">&deg;C</sup></B></p>";
    #endif
    ptr+=F("</table></form>");
    #ifdef debug1
    Serial.print(String(millis())+F(": www BODYWSTAW len: "));
    Serial.println(ptr.length());
    #endif
    return ptr.c_str();;
  }


  #ifdef debug
    Serial.print(F("End processor "));
//    Serial.println(var);
  #endif
  return String("\0").c_str();
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
String uptimedana(unsigned long started_local) {
  String wynik = " ";
  unsigned long  partia = millis() - started_local;
  if (partia<1000) return F("< 1 ") +String(t_sek)+" ";
  #ifdef debug
    Serial.print(F("Uptimedana: "));
  #endif
  if (partia >= 24 * 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (24 * 60 * 60 * 1000);
    partia -= podsuma * 24 * 60 * 60 * 1000;
    wynik += (String)podsuma + F(" ") +String(t_day)+F(" ");

  }
  if (partia >= 60 * 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 60 * 1000);
    partia -= podsuma * 60 * 60 * 1000;
    wynik += (String)podsuma + F(" ") +String(t_hour)+F(" ");
  }
  if (partia >= 60 * 1000 ) {
    unsigned long  podsuma = partia / (60 * 1000);
    partia -= podsuma * 60 * 1000;
    wynik += (String)podsuma + F(" ") +String(t_min)+F(" ");
    //Serial.println(podsuma);
  }
  if (partia >= 1 * 1000 ) {
    unsigned long  podsuma = partia / 1000;
    partia -= podsuma * 1000;
    wynik += (String)podsuma + F(" ")+String(t_sek)+F(" ");
    //Serial.println(podsuma);
  }
  #ifdef debug
    Serial.println(wynik);
  #endif
  //wynik += (String)partia + "/1000";  //pomijam to wartosci <1sek
  return wynik;
}




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
void subWebServers() {
  //  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  //  httpwebserver.on("/test", HTTP_GET, []() {
  //    httpwebserver.send(200, "text/html",  SendHTML(dcoThermstatometerS->getlast(), waterThermometerS->getlast())); //dcoThermstat,waterTherm));
  //    httpwebserver.sendHeader("Connection", "close");
  //  });
  wdt_reset();
  hw_wdt_disable();
  #ifdef debug
    Serial.println("subWerbServers...");
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
    request->send(200, "text/plain; charset=utf-8", String(F("<font color=\"Blue\">"))+String(coTherm,1)+String(+F("<font>"))); //dcoThermstatometerS->getlast());
  }).setAuthentication("", "");
  webserver.on(slashstr dwaterThermstat, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(F("<font color=\"Blue\">"))+String(waterTherm,1)+String(+F("<font>")));
  }).setAuthentication("", "");
  webserver.on(slashstr dpump1, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(getpumpstatus(1)));
  }).setAuthentication("", "");
  webserver.on(slashstr dpump2, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(getpumpstatus(2)));
  }).setAuthentication("", "");
  webserver.on(slashstr dNThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(NThermometerS->getlast(),1));
  }).setAuthentication("", "");
  webserver.on(slashstr dWThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(WThermometerS->getlast(),1));
  });
  webserver.on(slashstr dEThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(EThermometerS->getlast(),1));
  }).setAuthentication("", "");
  webserver.on(slashstr dSThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(SThermometerS->getlast(),1));
  });
  #ifdef newSensorT1
  webserver.on(slashstr dT1ThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(T1ThermometerS->getlast(),1));
  }).setAuthentication("", "");
  #endif
  webserver.on(slashstr dallThermometerS, HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", String(F("<font color=\"Green\">"))+String(OutsideTempAvg)+String(+F("<font>")));
  }).setAuthentication("", "");
  webserver.on(slashstr do_stopkawebsiteS , HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html; charset=utf-8", String(do_stopkawebsite()));
  }).setAuthentication("", "");
  webserver.on(slashstr , HTTP_GET, [](AsyncWebServerRequest * request) {
    //  httpwebserver.on("/", HTTP_GET, []() {
   // request.setAuthentication("", "");
    request->send_P(200, "text/html; charset=utf-8",  index_html, processor); //,processor
  }).setAuthentication("", "");
wdt_reset();
  //boolean webInit() {
  //  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {request->redirect("/update");});
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
  webserver.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });
  hw_wdt_enable();
}

//******************************************************************************************
void display_temp_rotation() {
  //return;
  float coTherm = coThermometerS->getlast();
  float waterTherm = waterThermometerS ->getlast();
  wdt_reset();
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
        display.print((String)"GAS " + int(gasCOMeterS->getlast()));
#ifdef debug
        Serial.println((String)"GAS " + int(gasCOMeterS->getlast()));
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
    wdt_reset();
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
  if (gasCOMeterS->getlast() > gas_leakage) {
    digitalWrite(buzzer, HIGH);
#ifdef debug
    Serial.print("Gas leakage: ");
    Serial.print(gasCOMeterS->getlast());
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

  #ifdef debug1
  Serial.println("Saving config...........................prepare ");
  #endif
  #ifdef enableWebSerial
  if (!starting) {WebSerial.println("Saving config...........................prepare ");}
  #endif
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
    #ifdef debug1
    Serial.println(String(millis())+": Saving config........................... to EEPROM some data changed");
    #endif
    #ifdef enableWebSerial
    if (!starting) {WebSerial.println(String(millis())+": Saving config........................... to EEPROM some data changed");}
    #endif

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
  #ifdef debug1
  Serial.println("saving energy used");
  #endif
  #ifdef enableWebSerial
  if (!starting) {WebSerial.println("saving energy used");}
  #endif
  unsigned int temp = 0;
  EEPROM.get(1,temp);
  if (temp != runNumber ) {EEPROM.put(1, runNumber);}
  double dtemp =0;
  EEPROM.get(1+sizeof(runNumber),dtemp);
  if (dtemp != energy1used) {EEPROM.put(1+sizeof(runNumber), energy1used);}
  EEPROM.get(1+sizeof(runNumber)+sizeof(energy1used),dtemp);
  if (dtemp != energy2used) {EEPROM.put(1+sizeof(runNumber)+sizeof(energy1used), energy2used);}
  EEPROM.commit();
  #ifdef debug
    Serial.println("config saved");
  #endif
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
  //long czaskonwersji = millis();
  #ifdef debug
    Serial.print("adspin: ");
    Serial.println(adspin);
  #endif
  if (isadslinitialised) {
    for (int i = 0 ; i <= 75 ; i++) //Monitors and logs the current input for 200 cycles to determine max and min current
    {
      wdt_reset();
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
  return RMSCurrent*0.707 ; //return real current nmqttident max rms
}

const char htmlup[] PROGMEM = R"rawliteral(
  <form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>)rawliteral";
void handleUpdate(AsyncWebServerRequest *request) {
  request->send(200, "text/html", htmlup);
}

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
//#define U_FLASH   0  //copied from ESP8266 library dla ESP32 sprawdzic
#define U_LittleFS  100
//#define U_AUTH    200
  if (!index) {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device rebomqttidents");
    response->addHeader("Refresh", "5");
    response->addHeader("Location", "/");
    request->send(response);
    Serial.println("Update");
    content_len = request->contentLength();
    // if filename includes LittleFS, update the LittleFS partition
    int cmd = (filename.indexOf("LittleFS") > -1) ? U_LittleFS : U_FLASH;
#ifdef ESP8266
    Update.runAsync(true);
    if (!Update.begin(content_len, cmd)) {
#else
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
#endif
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
#ifdef ESP8266
  } else {
    Serial.printf("Progress: %d%%\n", (Update.progress() * 100) / Update.size());
#endif
  }

  if (final) {


    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
      SaveEnergy();
      Serial.println("Update complete");
      Serial.flush();
      WiFi.forceSleepBegin();
      webserver.end();
      WiFi.disconnect();
//      wifi.disconnect();
      delay(5000);
      WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while (1)ESP.restart(); wdt_reset(); ESP.restart();
    }
  }
}

void printProgress(size_t prg, size_t sz) {
  Serial.printf("Progress: %d%%\n", (prg * 100) / content_len);
}

String getJsonVal(String json, String tofind)
{ //function to get value from json payload
  json.trim();
  tofind.trim();
  #ifdef debugweb
  WebSerial.println("json0: "+json);
  #endif
  if (!json.isEmpty() and !tofind.isEmpty() and json.startsWith("{") and json.endsWith("}"))  //check is starts and ends as json data and nmqttident null
  {
    json=json.substring(1,json.length()-1);                             //cut start and end brackets json
    #ifdef debugweb
    WebSerial.println("json1: "+json);
    #endif
    int tee=0; //for safety ;)
    #define maxtee 500
    while (tee!=maxtee)
    {         //parse all nodes
      int pos = json.indexOf(",",1);                //position to end of node:value
      if (pos==-1) {tee=maxtee;}
      String part;
      if (pos>-1) {part = json.substring(0,pos);} else {part=json; }       //extract parameter node:value
      part.replace("\"","");                      //clean from text indent
      part.replace("'","");
      json=json.substring(pos+1);                      //cut input for extracted node:value
      if (part.indexOf(":",1)==-1) {
        #ifdef debugweb
        WebSerial.println("Return no : data");
        #endif
        break;
      }
      String node=part.substring(0,part.indexOf(":",1));    //get node name
      node.trim();
      String nvalue=part.substring(part.indexOf(":",1)+1); //get node value
      nvalue.trim();
      #ifdef debugweb
      WebSerial.println("jsonx: "+json);
      WebSerial.println("tee: "+String(tee)+" tofind: "+tofind+" part: "+part+" node: "+node +" nvalue: "+nvalue + " indexof , :"+String(json.indexOf(",",1)));
      #endif
      if (tofind==node)
      {
         #ifdef debugweb
         WebSerial.println("Found node return val");
         #endif
        return nvalue;
        break;
      }
      tee++;
      #ifdef debugweb
      delay(1000);
      #endif
      if (tee>maxtee) {
        #ifdef debugweb
         WebSerial.println("tee exit: "+String(tee));
        #endif
        break;  //safety bufor
      }
    }
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+": Json "+json+"  No mqttident contain searched value of "+tofind);
    #endif
  } else
  {
    #ifdef enableWebSerial
    WebSerial.println(String(millis())+": Inproper Json format or null: "+json+" to find: "+tofind);
    #endif
  }
  return "\0";
}

#ifdef enableWebSerial
void recvMsg(uint8_t *data, size_t len)
{ // for WebSerial
  WebSerial.print(String(millis())+": ");
  WebSerial.println(F("Received Data on WebSerial..."));
  String d = "";
  for (size_t i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  d.toUpperCase();
  WebSerial.println("Received: " + String(d));

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
    reconnect();
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
  if (d == "HELP")
  {
    WebSerial.print(String(millis())+": ");
    WebSerial.println(F("KOMENDY:\n \
      FORCECOBELOW xx  -Zmienia wartość xx 'wymusza pompe CO poniżej temperatury średniej zewnetrznej', \n \
      COCUTOFFTEMP xx  -Zmienia wartość xx 'Temperatura graniczna na wymienniku oznacza ze piec sie grzeje',\n \
      HIST xx          -Zmienia wartość xx histerezy progu grzania,\n \
      ForceCO 0/1      -1(ON) -Wymusza grzanie CO 0(OFF) -deaktywuje,\n \
      ForceWater 0/1   -1(ON) -Wymusza grzanie Wody 0(OFF) -deaktywuje,\n \
      RESTART          -Uruchamia ponownie układ,\n \
      RECONNECT        -Dokonuje ponownej próby połączenia z bazami,\n \
      SAVE             -Wymusza zapis konfiguracji,\n \
      RESET_CONFIG     -UWAGA!!!! Resetuje konfigurację do wartości domyślnych"));
  }
}
#endif

void restart()
{
  delay(1500);
  WiFi.forceSleepBegin();
  webserver.end();
  WiFi.disconnect();
//      wifi.disconnect();
  delay(5000);
  WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while (1)ESP.restart(); wdt_reset(); ESP.restart();
}

bool PayloadStatus(String payloadStr, bool state)
{
  payloadStr.toUpperCase();
  payloadStr.trim();
  payloadStr.replace(",", ".");      //for localization correction
  if (state and (payloadStr == "ON" or payloadStr == "TRUE" or payloadStr == "START" or payloadStr == "1"  or payloadStr == "ENABLE" or payloadStr == "HEAT")) return true;
  else
  if (!state and (payloadStr == "OFF" or payloadStr == "FALSE" or payloadStr == "STOP" or payloadStr == "0" or payloadStr == "DISABLE")) return true;
  else return false;
}
bool PayloadtoValidFloatCheck(String payloadStr)
{
  if (PayloadtoValidFloat(payloadStr) == InitTemp) return false; else return true;
}

float PayloadtoValidFloat(String payloadStr, bool withtemps_minmax, float mintemp, float maxtemp)  //bool withtemps_minmax=false, float mintemp=InitTemp,float
{
  payloadStr.trim();
  payloadStr.replace(",", ".");
  float valuefromStr = payloadStr.toFloat();
  if (isnan(valuefromStr) || !isValidNumber(payloadStr))
  {
    #ifdef debug
    Serial.println(F("Value is nmqttident a valid number, ignoring..."));
    #endif
    #ifdef enableWebSerial
    WebSerial.print(String(millis())+": ");
    WebSerial.println(F("Value is nmqttident a valid number, ignoring..."));
    #endif
    return InitTemp;
  } else
  {
    if (!withtemps_minmax)
    {
      return valuefromStr;
    } else {
      #ifdef debug
      Serial.println("Value is valid number: "+String(valuefromStr,2));
      #endif
      #ifdef enableWebSerial
      WebSerial.print(String(millis())+": ");
      WebSerial.println("Value is valid number: "+String(valuefromStr,2));
      #endif
      if (valuefromStr>maxtemp and maxtemp!=InitTemp) valuefromStr = maxtemp;
      if (valuefromStr<mintemp and mintemp!=InitTemp) valuefromStr = mintemp;
      return valuefromStr;
    }
  }
}


void ReadTemperatures()
{
  // bool assignedsensor=false;
  // UnassignedTempSensor="";
  //  String addrstr = "";
  // //read temperatures
  // #ifdef debug
  // Serial.println(F("...Reading 1wire sensors..."));
  // #endif
  // sensors.setWaitForConversion(true); //
  // sensors.requestTemperatures();        //Send the command to get temperatures
  // sensors.setWaitForConversion(false); // switch to async mode
  // uint8_t  addr[8];
  // float temp1w;
  // int count = sensors.getDS18Count();
  // if (count==0) count = maxsensors;
  // #ifdef debug
  // Serial.println("Sensors new: "+String(count));
  // #endif
  // for (int j = 0; j < count; j++) {
  //     temp1w = sensors.getTempCByIndex(j);
  //     addrstr="";
  //     sensors.getAddress(addr, j);
  //     for (int i1 = 0; i1 < 8; i1++) if (String(addr[i1], HEX).length() == 1) addrstr += "0" + String(addr[i1], HEX); else addrstr += String(addr[i1], HEX); //konwersja HEX2StringHEX
  //     //zapisanie do zmiennej addr[8], addrstr, aktualiozacja index wskazany przez j i aktualozacja temp1w.
  //     for (int z = 0; z < maxsensors; z++) {
  //       #ifdef debug
  //       if (z < 10) Serial.print(String(" "));
  //       Serial.print(String(z)+": ");
  //       #endif
  //       if (array_cmp_8(room_temp[z].addressHEX, addr, sizeof(room_temp[z].addressHEX) / sizeof(room_temp[z].addressHEX[0]),sizeof(addr) / sizeof(addr[0])) == true) {
  //         assignedsensor=true;
  //         if (temp1w!=DS18B20nodata) TempSensor[z].tempread = temp1w;
  //       }
  //     }
  //     if (!assignedsensor and UnassignedTempSensor.indexOf(addrstr)==-1) UnassignedTempSensor += ";"+String(addrstr)+" "+String(temp1w);
  //     assignedsensor=false;
  //     #ifdef debug
  //     Serial.println((String(millis())+": "+j)+":  Collected ROM=: " + addrstr + "  Temp.: "+String(temp1w));
  //     #endif
  //     #ifdef enableWebSerial
  //     WebSerial.println(String(millis())+": "+String(j)+":  Collected ROM=: " + addrstr + "  Temp.: "+String(temp1w));
  //     #endif
  //   }

  // #ifdef debug
  // Serial.println("Unassigned Sensors: "+UnassignedTempSensor);
  // #endif
  // #ifdef enableWebSerial
  // WebSerial.println(String(millis())+": "+"Unassigned Sensors: "+UnassignedTempSensor);
  // #endif
}

String convertPayloadToStr(byte *payload, unsigned int length)
{
  char s[length + 1];
  s[length] = 0;
  for (unsigned int i = 0; i < length; ++i)
    s[i] = payload[i];
  String tempRequestStr(s);
  return tempRequestStr;
}

bool isValidNumber(String str)
{
  bool valid = true;
  for (byte i = 0; i < str.length(); i++)
  {
    char ch = str.charAt(i);
    valid &= isDigit(ch) ||
             ch == '+' || ch == '-' || ch == ',' || ch == '.' ||
             ch == '\r' || ch == '\n';
  }
  return valid;
}

int mqttcount =0;
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  const String topicStr(topic);

  String payloadStr = convertPayloadToStr(payload, length);
mqttcount++;

//get Voltage on same phase
  if (topicStr == SUPLA_VOLT_TOPIC)
  {
    String ident = String(millis())+F(": electricmain Volt ");
    #ifdef debug
    Serial.print(ident);
    #endif
    #ifdef enableWebSerial
    if (mqttcount%10 == 0 ) WebSerial.print(ident);
    #endif
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
    #ifdef debug
    Serial.print(ident);
    #endif
    #ifdef enableWebSerial
    if (mqttcount%10 == 0 )WebSerial.print(ident);
    #endif
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
    String ident = String(millis())+F(": Boiler PUMP1 Wather ");
    payloadStr.toUpperCase();
    #ifdef debug1
    Serial.print(ident + "Set mode: ");
    #endif
    #ifdef enableWebSerial
    WebSerial.print(ident + "Set mode: ");
    #endif
    bool tmp = PayloadStatus(payloadStr, true) && !PayloadStatus(payloadStr, false);
    if (tmp != prgstatusrelay1WO) receivedmqttdata = true;
    if (PayloadStatus(payloadStr, true))
    {
      ownother=F("<B> MQTT pump1WO ")+payloadStr+F("</B>");
      prgstatusrelay1WO = true;
      forceWater = true;
      forceCO = false;
      najpierwCO = false;
      #ifdef enableWebSerial
      WebSerial.println("WO mode " + payloadStr);
      #endif
      #ifdef debug1
      Serial.println("WO mode " + payloadStr);
      #endif
    }
    else if (PayloadStatus(payloadStr, false))
    {
      prgstatusrelay1WO = false;
      forceWater = false;
      forceCO = false;
      najpierwCO = false;
      #ifdef enableWebSerial
      WebSerial.println("WO mode " + payloadStr);
      #endif
      #ifdef debug1
      Serial.println("WO mode " + payloadStr);
      #endif
    }
    else {
      #ifdef debug1
      Serial.println(ident + " unknkown ");
      #endif
      #ifdef enableWebSerial
      WebSerial.println(ident + " unknkown ");
      #endif
    }
  } else
//PUMP2
  if (topicStr == BOILERROOM_SWITCH_TOPIC_SET+"_"+BOILERROOM_PUMP2CO)
  {
    String ident = String(millis())+F(": Boiler PUMP2 CO ");
    payloadStr.toUpperCase();
    #ifdef debug1
    Serial.print(ident + "Set mode: ");
    #endif
    #ifdef enableWebSerial
    WebSerial.print(ident + "Set mode: ");
    #endif
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

      #ifdef debug1
      Serial.println("CO mode " + payloadStr);
      #endif
      #ifdef enableWebSerial
      WebSerial.println("CO mode " + payloadStr);
      #endif
    }
    else if (PayloadStatus(payloadStr, false))
    {
      prgstatusrelay2CO = false;
      forceCO = false;
      forceWater = false;
      najpierwCO = false;
      #ifdef enableWebSerial
      WebSerial.println("CO mode " + payloadStr);
      #endif
      #ifdef debug1
      Serial.println("CO mode " + payloadStr);
      #endif
    }
    else {
      #ifdef debug1
      Serial.println(ident + " unknkown ");
      #endif
      #ifdef enableWebSerial
      WebSerial.println(ident + " unknkown ");
      #endif
    }
  }


// //    Serial.print("  forceCO: ");    Serial.println((forceCO? "LOW" : "HIGH" ));
// //    Serial.print(" forceWO: "); Serial.println((forceWater? "LOW" : "HIGH" ));
// //    Serial.print(" prgstatusrelay2CO: ");     Serial.println((prgstatusrelay2CO? "LOW" : "HIGH" ));
// //    Serial.print(" prgstatusrelay1WO: ");             Serial.println((prgstatusrelay1WO? "LOW" : "HIGH" ));
// //    Serial.println("******************************************");
//     }
//     if (String(topic) == String(String(me_lokalizacja) + "/switch/" + PrintHex8(mac, '\0' , sizeof(mac) / sizeof(mac[0])) + "/pump2CO/cmd")) {
//       ownother="<B> MQTT pump2CO "+paylstr+"</B>";
//       if ( paylstr == "ON" )  {
//         #ifdef debug
//           Serial.print("1111ff11111");
//         #endif
//       }
//       if (paylstr == "OFF" ) {
//         }
//  //     if (memcmp(payload,"ON",sizeof(length))==0) { forceCO = true; Serial.print("1111ff11111"); } else { forceCO = false; Serial.print("00000ff00000"); }
//     #ifdef debug
//       Serial.print("*2CO*************************************************Statechanged: ");
//     #endif
// //    Serial.println((const char*)payload);
// //    Serial.print(sizeof(*payload));
// //    Serial.print(" forceCO: ");    Serial.print((forceCO? "LOW" : "HIGH" ));
// //    Serial.print(" forceWO: "); Serial.print((forceWater? "LOW" : "HIGH" ));
// //    Serial.print(" prgstatusrelay2CO: ");     Serial.print((prgstatusrelay2CO? "LOW" : "HIGH" ));
// //    Serial.print(" prgstatusrelay1WO: ");             Serial.print((prgstatusrelay1WO? "LOW" : "HIGH" ));
// //    Serial.println("******************************************");

//       }

    receivedmqttdata = false;
}

void reconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected() and mqtt_offline_retrycount < mqtt_offline_retries)
  {
    #ifdef debug
    Serial.print(F("Attempting MQTT connection..."));
    #endif
    #ifdef enableWebSerial
    WebSerial.print(String(millis())+": "+F("Attempting MQTT connection..."));
    #endif
    const char *clientId = me_lokalizacja.c_str();

    if (mqttClient.connect(clientId, mqtt_user, mqtt_password))
    {
      #ifdef debug
      Serial.println(F("ok"));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(F("ok"));
      #endif
      mqtt_offline_retrycount = 0;

      mqttClient.subscribe(SUPLA_VOLT_TOPIC.c_str());
      mqttClient.subscribe(SUPLA_FREQ_TOPIC.c_str());
      mqttClient.subscribe((BOILERROOM_SWITCH_TOPIC_SET+"_"+BOILERROOM_PUMP1WA).c_str());
      mqttClient.subscribe((BOILERROOM_SWITCH_TOPIC_SET+"_"+BOILERROOM_PUMP2CO).c_str());




      // for (int x=0;x<maxsensors;x++){
      //   //String mqttident=getmqttident(x);
      //   if (room_temp[x].idpinout!=0) client.subscribe((ROOM_TEMPERATURE_SETPOINT_SET_TOPIC + getmqttident(x) + SET_LAST).c_str());
      // }
      // mqttClient.subscribe(NEWS_GET_TOPIC.c_str());
      // mqttClient.subscribe(COPUMP_GET_TOPIC.c_str());
      // mqttClient.subscribe(BOILER_FLAME_STATUS_TOPIC.c_str());

      // mqttClient.subscribe(TEMP_CUTOFF_SET_TOPIC.c_str());  //tego nie ma w obsludze

    } else {
      #ifdef debug
      Serial.print(F(" failed, rc="));
      #endif
      #ifdef enableWebSerial
      WebSerial.print(F(" failed, rc="));
      #endif
      #ifdef debug
      Serial.print(mqttClient.state());
      #endif
      #ifdef enableWebSerial
      WebSerial.print(mqttClient.state());
      #endif
      #ifdef debug
      Serial.println(F(" try again in 5 seconds"));
      #endif
      #ifdef enableWebSerial
      WebSerial.println(F(" try again in 5 seconds"));
      #endif
      mqtt_offline_retrycount++;
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

#ifdef ENABLE_INFLUX
void updateInfluxDB()
{
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
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP1WA_E), pump1energyS->getlast(energy_energyUsed));
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP2CO_E), pump2energyS->getlast(energy_energyUsed));
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP1WA_E)+"_Current", pump1energyS->getlast(energy_current));
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP2CO_E)+"_Current", pump2energyS->getlast(energy_current));
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP1WA_E)+"_Voltage", pump1energyS->getlast(energy_voltage));
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP2CO_E)+"_Voltage", pump2energyS->getlast(energy_voltage));
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP1WA_E)+"_Freq", pump1energyS->getlast(energy_freq));
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP2CO_E)+"_Freq", pump2energyS->getlast(energy_freq));
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP1WA_E)+"_Power", pump1energyS->getlast(energy_power));
  InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP2CO_E)+"_Power", pump2energyS->getlast(energy_power));
    InfluxSensor.addField(mqttident + String(BOILERROOM_PUMP2CO), prgstatusrelay2CO?"1":"0");
  if (NThermometerS->getlast() != InitTemp) {InfluxSensor.addField(mqttident + String(OUTSIDE_TEMPERATURE_N), NThermometerS->getlast());}
  if (EThermometerS->getlast() != InitTemp) {InfluxSensor.addField(mqttident + String(OUTSIDE_TEMPERATURE_E), EThermometerS->getlast());}
  if (WThermometerS->getlast() != InitTemp) {InfluxSensor.addField(mqttident + String(OUTSIDE_TEMPERATURE_W), WThermometerS->getlast());}
  if (SThermometerS->getlast() != InitTemp) {InfluxSensor.addField(mqttident + String(OUTSIDE_TEMPERATURE_S), SThermometerS->getlast());}
  #ifdef newSensorT1
  InfluxSensor.addField(mqttident + String(BOILERROOM_TEMPERATURE_T1), T1ThermometerS->getlast());
  #endif
  if (OutsideTempAvg != InitTemp) {InfluxSensor.addField(mqttident + String(OUTSIDE_TEMPERATURE_A), OutsideTempAvg);}


  // InfluxSensor.addField(String(TEMP_CUTOFF)+String(kondygnacja), cutOffTemp);
//  InfluxSensor.addField(String(DIAGS_mqttidentHERS_FAULT), status_Fault ? "1" : "0");
//  InfluxSensor.addField(String(DIAGS_mqttidentHERS_DIAG), status_Diagnostic ? "1" : "0");
  // InfluxSensor.addField(String(INTEGRAL_ERROR_GET_TOPIC)+String(kondygnacja), ierr);
  // InfluxSensor.addField(String(LOG_GET_TOPIC)+String(kondygnacja), LastboilerResponseError);

  // Print what are we exactly writing
  #ifdef enableWebSerial
  WebSerial.println(String(millis())+": "+("Writing to InfluxDB: "));
//  WebSerial.println(InfluxClient.pointToLinePrmqttidentocol(InfluxSensor));
  #endif
  // Write point
  if (!InfluxClient.writePoint(InfluxSensor))
  {
    #ifdef debug
    Serial.print(String(millis())+": "+("InfluxDB write failed: "));
    Serial.println(InfluxClient.getLastErrorMessage());
    #endif
    #ifdef enableWebSerial
    WebSerial.print(String(millis())+": "+("InfluxDB write failed: "));
    WebSerial.println(InfluxClient.getLastErrorMessage());
    #endif
  }
}
#endif

void mqttHAPublish_Config (String HADiscoveryTopic, String ValueTopicName, String SensorName, String friendlySensorName, int unitClass, String cmd_temp){
  String HAClass;
  switch (unitClass) {
    case mqtt_HAClass_temperature:   HAClass = F("\"dev_cla\":\"temperature\",\"unit_of_meas\": \"°C\",\"ic\": \"mdi:thermometer\","); break;
    case mqtt_HAClass_pressure:      HAClass = F("\"dev_cla\":\"pressure\",\"unit_of_meas\": \"hPa\",\"ic\": \"mdi:mdiCarSpeedLimiter\",");  break; //cbar, bar, hPa, inHg, kPa, mbar, Pa, psi
    case mqtt_HAClass_humidity:      HAClass = F("\"dev_cla\":\"humidity\",\"unit_of_meas\": \"%\",\"ic\": \"mdi:mdiWavesArrowUp\","); break;
    case mqtt_HAClass_energy:        HAClass = F("\"dev_cla\":\"energy\",\"unit_of_meas\": \"kWh\",\"state_class\":\"total_increasing\",\"ic\": \"mdi:lightning-bolt-circle\","); break; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
    case mqtt_HAClass_power:         HAClass = F("\"dev_cla\":\"power\",\"unit_of_meas\": \"W\",\"ic\": \"mdi:alpha-W-box\","); break; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
    case mqtt_HAClass_voltage:       HAClass = F("\"dev_cla\":\"voltage\",\"unit_of_meas\": \"V\",\"ic\": \"mdi:alpha-v-box\","); break; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
    case mqtt_HAClass_current:       HAClass = F("\"dev_cla\":\"current\",\"unit_of_meas\": \"A\",\"ic\": \"mdi:alpha-a-box\","); break; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
    case mqtt_HAClass_freq:          HAClass = F("\"dev_cla\":\"frequency\",\"unit_of_meas\": \"Hz\",\"ic\": \"mdi:sine-wave\","); break; //energy: Wh, kWh, MWh	Energy, statistics will be stored in kWh. Represents power over time. Nmqttident to be confused with power.  power: W, kW	Power, statistics will be stored in W.
    case mqtt_HAClass_high:          HAClass = F("\"unit_of_meas\": \"m\",\"ic\": \"mdi:speedometer-medium\","); break;
    case mqtt_HAClass_co:            HAClass = F("\"dev_cla\":\"carbon_monoxide\",\"unit_of_meas\": \"ppm\",\"ic\": \"mdi:molecule-co\","); break;
    case mqtt_HAClass_switch:        HAClass = F("\"pl_off\":\"OFF\",\"pl_on\":\"ON\",");
                                     if (cmd_temp.length()>0) {HAClass +=F("\"cmd_t\":\"") + cmd_temp + F("_") + SensorName + F("\",");} break;
    default:                         HAClass = "\0"; break;
  }
  mqttClient.publish((HADiscoveryTopic + F("_") + mqttident + SensorName + F("/config")).c_str(), (F("{\"name\":\"") + mqttident + friendlySensorName + F("\",\"uniq_id\": \"") + mqttident + SensorName + F("\",\"stat_t\":\"") + ValueTopicName + F("\",\"val_tpl\":\"{{value_json.") + mqttident + SensorName +F("}}\",") + HAClass + F("\"qos\":") + mqttQOS + F(",") + mqttdeviceid + F("}")).c_str(), mqtt_Retain);
}

void updateMQTTData() {
  const String payloadvalue_startend_val = F("\0"); // value added before and after value send to mqtt queue
  mqttClient.setBufferSize(2048);


  String tmpbuilder = F("{");
  tmpbuilder += F("\"rssi\":")+ String(WiFi.RSSI());
  tmpbuilder += F(",\"CRT\":")+ String(runNumber);
  tmpbuilder += F(",\"uptime\":")+ String((millis())/1000);   //w sekundach

  if (NThermometerS->getlast() != InitTemp) {tmpbuilder += F(",\"") + mqttident + OUTSIDE_TEMPERATURE_N + F("\": ") + payloadvalue_startend_val + String(NThermometerS->getlast()) + payloadvalue_startend_val;}
  if (EThermometerS->getlast() != InitTemp) {tmpbuilder += F(",\"") + mqttident + OUTSIDE_TEMPERATURE_E + F("\": ") + payloadvalue_startend_val + String(EThermometerS->getlast()) + payloadvalue_startend_val;}
  if (WThermometerS->getlast() != InitTemp) {tmpbuilder += F(",\"") + mqttident + OUTSIDE_TEMPERATURE_W + F("\": ") + payloadvalue_startend_val + String(WThermometerS->getlast()) + payloadvalue_startend_val;}
  if (SThermometerS->getlast() != InitTemp) {tmpbuilder += F(",\"") + mqttident + OUTSIDE_TEMPERATURE_S + F("\": ") + payloadvalue_startend_val + String(SThermometerS->getlast()) + payloadvalue_startend_val;}
  if (OutsideTempAvg != InitTemp) {tmpbuilder += F(",\"") + mqttident + OUTSIDE_TEMPERATURE_A + F("\": ") + payloadvalue_startend_val + String(OutsideTempAvg) + payloadvalue_startend_val;}
  #ifdef newSensorT1
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_TEMPERATURE_T1 + F("\": ") + payloadvalue_startend_val + String(T1ThermometerS->getlast()) + payloadvalue_startend_val;
  #endif
  if (coTherm != InitTemp) {tmpbuilder += F(",\"") + mqttident + HEATERCO_TEMPERATURE + F("\": ") + payloadvalue_startend_val + String(coTherm) + payloadvalue_startend_val;}
  if (waterTherm != InitTemp) {tmpbuilder += F(",\"") + mqttident + WATER_TEMPERATURE + F("\": ") + payloadvalue_startend_val + String(waterTherm) + payloadvalue_startend_val;}
  if (bmTemp != InitTemp) {tmpbuilder += F(",\"") + mqttident + BOILERROOM_TEMPERATURE + F("\": ") + payloadvalue_startend_val + String(bmTemp) + payloadvalue_startend_val;}
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PRESSURE + F("\": ") + payloadvalue_startend_val + String(dbmpressval) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_HIGH + F("\": ") + payloadvalue_startend_val + String(bm_high) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_HIGHREAL + F("\": ") + payloadvalue_startend_val + String(bm_high_real) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_COVAL + F("\": ") + payloadvalue_startend_val + String(dcoval) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP1WA_E + F("\": ") + payloadvalue_startend_val + String(pump1energyS->getlast(energy_energyUsed)) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP2CO_E + F("\": ") + payloadvalue_startend_val + String(pump2energyS->getlast(energy_energyUsed)) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP1WA_E + F("_Current\": ") + payloadvalue_startend_val + String(pump1energyS->getlast(energy_current)) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP2CO_E + F("_Current\": ") + payloadvalue_startend_val + String(pump2energyS->getlast(energy_current)) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP1WA_E + F("_Freq\": ") + payloadvalue_startend_val + String(pump1energyS->getlast(energy_freq)) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP2CO_E + F("_Freq\": ") + payloadvalue_startend_val + String(pump2energyS->getlast(energy_freq)) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP1WA_E + F("_Voltage\": ") + payloadvalue_startend_val + String(pump1energyS->getlast(energy_voltage)) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP2CO_E + F("_Voltage\": ") + payloadvalue_startend_val + String(pump2energyS->getlast(energy_voltage)) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP1WA_E + F("_Power\": ") + payloadvalue_startend_val + String(pump1energyS->getlast(energy_power)) + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP2CO_E + F("_Power\": ") + payloadvalue_startend_val + String(pump2energyS->getlast(energy_power)) + payloadvalue_startend_val;
  //pump status
  mqttClient.publish(BOILERROOM_SENSOR_TOPIC.c_str(),(tmpbuilder+F("}")).c_str(), mqtt_Retain);
  tmpbuilder = F("{");
  tmpbuilder += F("\"CRT\":")+ String(runNumber);
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP1WA + F("\": ") + payloadvalue_startend_val + String(prgstatusrelay1WO?"\"ON\"":"\"OFF\"") + payloadvalue_startend_val;
  tmpbuilder += F(",\"") + mqttident + BOILERROOM_PUMP2CO + F("\": ") + payloadvalue_startend_val + String(prgstatusrelay2CO?"\"ON\"":"\"OFF\"") + payloadvalue_startend_val;   //getpumpstatus(2)
  mqttClient.publish(BOILERROOM_SWITCH_TOPIC.c_str(),(tmpbuilder+F("}")).c_str(), mqtt_Retain);


  publishhomeassistantconfig++; // zwiekszamy licznik wykonan wyslania mqtt by co publishhomeassistantconfigdivider wysłań wysłać autoconfig discovery dla homeassisatnt
  if (publishhomeassistantconfig % publishhomeassistantconfigdivider == 0)
  {
    //Make Homeassistant autodiscovery and autoconfig
    //Temperatures
    if (NThermometerS->getlast()!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_N, dNThermometerS, mqtt_HAClass_temperature);
    if (EThermometerS->getlast()!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_E, dEThermometerS, mqtt_HAClass_temperature);
    if (WThermometerS->getlast()!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_W, dWThermometerS, mqtt_HAClass_temperature);
    if (SThermometerS->getlast()!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_S, dSThermometerS, mqtt_HAClass_temperature);
    if (OutsideTempAvg!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, OUTSIDE_TEMPERATURE_A, dallThermometerS, mqtt_HAClass_temperature);
    #ifdef newSensorT1
    if (T1ThermometerS->getlast()!=InitTemp) mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_TEMPERATURE_T1, dT1ThermometerS, mqtt_HAClass_temperature);
    #endif
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
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP1WA_E+F("_Current"), String(dpump1energyS)+F("_Current"), mqtt_HAClass_current);
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP2CO_E+F("_Current"), String(dpump2energyS)+F("_Current"), mqtt_HAClass_current);
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP1WA_E+F("_Voltage"), String(dpump1energyS)+F("_Voltage"), mqtt_HAClass_voltage);
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP2CO_E+F("_Voltage"), String(dpump2energyS)+F("_Voltage"), mqtt_HAClass_voltage);
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP1WA_E+F("_Freq"), String(dpump1energyS)+F("_Freq"), mqtt_HAClass_freq);
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP2CO_E+F("_Freq"), String(dpump2energyS)+F("_Freq"), mqtt_HAClass_freq);
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP1WA_E+F("_Power"), String(dpump1energyS)+F("_Power"), mqtt_HAClass_power);
    mqttHAPublish_Config(BOILERROOM_HA_SENSOR_TOPIC, BOILERROOM_SENSOR_TOPIC, BOILERROOM_PUMP2CO_E+F("_Power"), String(dpump2energyS)+F("_Power"), mqtt_HAClass_power);
    //pumps switch/state
    mqttHAPublish_Config(BOILERROOM_HA_SWITCH_TOPIC, BOILERROOM_SWITCH_TOPIC, BOILERROOM_PUMP1WA, dpump1, mqtt_HAClass_switch, BOILERROOM_SWITCH_TOPIC_SET);
    mqttHAPublish_Config(BOILERROOM_HA_SWITCH_TOPIC, BOILERROOM_SWITCH_TOPIC, BOILERROOM_PUMP2CO, dpump2, mqtt_HAClass_switch, BOILERROOM_SWITCH_TOPIC_SET);
  }

  // }
  #ifdef debug
  Serial.println(String(millis())+F(": MQTT Data Sended..."));
  #endif
  #ifdef enableWebSerial
  WebSerial.println(String(millis())+F(": MQTT Data Sended..."));
  #endif

}
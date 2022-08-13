/*server.
  Topic structure is built with get/set mechanizm for
  compatibility with home assistant and to allow external
  device control
  _GET_ topics are used to publish current thermostat state
  _SET_ topics are used to control the thermostat

  Works also offline.

*/

// v.2.0 Initial after move from Arduino Mega to ESP32

//#define tmplok "_TMP"

#define debug		//Serial Debug
//#define debug1
//#define debuglm



//     https://maximeborges.github.io/esp-stacktrace-decoder/


#define kondygnacja 0

//#define newSensorT1 //dodatkowy sesnor
#define ATOMIC_FS_UPDATE
#define MFG "MARM.pl Sp. z o.o."
#define wwwport 80
#define PL_lang
#define ENABLE_INFLUX        //if defined sending to influx database is performed at time when mqtt messages is send
//#define enableMQTT        //by pubsub library but problems with connect status
#define enableMQTTAsync
//#define enableWebSerial //webserial interface i've added own iimplementation in websocketlog
#define enableArduinoOTA
#define enableWebSocketlog
#define enableWebSocket
#define enableDebug2Serial    //if not enabled there is no way to output logs to Serial

//#define wdtreset
//#define enableMESHNETWORK
//#define enableWifiManager //not implemented at all
#define avSensorReadTime 2*60*1000 //sredni czas odczytu czujnikow bazowo 18b20 jest na sztywno
#define NEWSnode 1      //if defined -remove news topics config


//#boiler_gas_conversion_to_m3  1

#include "Common_symlinkFiles\sensivity-config-data.h" //it have definitions of sensivity data
#include "Common_symlinkFiles\config-translate.h" //definitions polish/english translate
#ifndef SSID_Name
#define SSID_Name "SSID_Name"
#endif
#ifndef SSID_PAssword
#define SSID_PAssword "SSID_PAssword"
#endif
#ifndef MQTT_username
#define MQTT_username "MQTT_username"
#endif
#ifndef MQTT_Password_data
#define MQTT_Password_data "MQTT_Password_data"
#endif
#ifndef MQTT_port_No
#define MQTT_port_No 1883     //default mqtt port
#endif
#ifndef MQTT_servername
#define MQTT_servername "MQTT_servername"     //default mqtt port
#endif

#ifdef ENABLE_INFLUX
#ifndef INFLUXDB_URL
#define INFLUXDB_URL "http://localhost:8086"
#endif
// InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>) but I use only version 1 as default in HomeAssistant
#ifndef INFLUXDB_DB_NAME
#define INFLUXDB_DB_NAME "test"
#endif
#ifndef INFLUXDB_USER
#define INFLUXDB_USER "test"
#endif
#ifndef INFLUXDB_PASSWORD
#define INFLUXDB_PASSWORD "test"
#endif
#define InfluxMeasurments "MARMpl_Measurments"
#endif

#define InitTempst 255
#define maxsensors 7            //maksymalna liczba czujnikow w tabeli
#define namelength 15 //ilosc znakow z nazwy czunika

// Your WiFi credentials.
// Set password to "" for open networks.




#define sensitive_size 32
#define sensitive_sizeS "32"



//*************************************************
//#define update_username "admin"
//#define update_password "admin"
#define slashstr "/"
#define update_path "/update"






    //  initialConfig = false,

    //
    //
    //
    //

// char WSSID[31],
    //  WPass[51],

    //  ssid[sensitive_size] = SSID_Name,
    //  pass[sensitive_size] = SSID_PAssword,
    //  mqtt_server[sensitive_size*2] = MQTT_servername,   // Your MQTT broker address and credentials
    //  mqtt_user[sensitive_size] = MQTT_username,
    //  mqtt_password[sensitive_size] = MQTT_Password_data;



//


//
      //  checkpumptime, //helper to count time to save energy




// unsigned int runNumber = 0, // count of restarts
//              publishhomeassistantconfig = 4,                               // licznik wykonan petli -strat od 0
//              publishhomeassistantconfigdivider = 5;                        // publishhomeassistantconfig % publishhomeassistantconfigdivider -publikacja gdy reszta z dzielenia =0 czyli co te ilosc wykonan petli opoznionej update jest wysylany config

//


// const unsigned long wifi_checkDelay = 30000,
//                     mqtt_offline_reconnect_after_ms = 15 * 60 * 1000,      // time when mqtt is offline to wait for next reconnect (15minutes)

//
//

 //unsigned long wifimilis,
//
//               lastmqtt_reconnect = 0,



// String ownother, UnassignedTempSensor;


/*
   current temperature topics
   if setter is used - thermostat works with external values, bypassing built-in sensor
   if no values on setter for more than 1 minute - thermostat falls back to built-in sensor
*/
//definition of parameter val of energy class
#define energy_current 0
#define energy_energyUsed 1
#define energy_power 2
#define energy_voltage 3
#define energy_freq 4
//definitions of parameter to mqtt publish sensor class
#define mqtt_HAClass_switch 0
#define mqtt_HAClass_temperature 1
#define mqtt_HAClass_pressure 2
#define mqtt_HAClass_humidity 3
#define mqtt_HAClass_energy 4
#define mqtt_HAClass_power 5
#define mqtt_HAClass_voltage 6
#define mqtt_HAClass_current 7
#define mqtt_HAClass_freq 8
#define mqtt_HAClass_high 9
#define mqtt_HAClass_co 10

// const int mqtt_Retain = 1;
// const String tempicon="<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
// const String presicon="<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
// const String attiicon="<i class=\"fas fa-water\" style=\"color:#90add6;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
// const String ppmicon="<i class=\"fas fa-tint\" style=\"color:#50ad00;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
// const String pumpicon="<i class=\"fas fa-tint\" style=\"color:#500d00;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
// const String humidicon="<i class=\"fas fa-humidity\" style=\"color:blue;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";


//const String mqttident = "CO"+String(kondygnacja)+"_";





// #include <EEPROM.h>

// #define CONFIG_VERSION "V01" sensitive_sizeS

// // Where in EEPROM?
// #define CONFIG_START 1+sizeof(runNumber)+sizeof(energy1used)*2+16

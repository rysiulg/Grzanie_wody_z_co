/*server.
  Topic structure is built with get/set mechanizm for
  compatibility with home assistant and to allow external
  device control
  _GET_ topics are used to publish current thermostat state
  _SET_ topics are used to control the thermostat

  Works also offline.

*/

// v.2.0 Initial after move from Arduino Mega to ESP32

#define debug		//Serial Debug
//#define debug1
//#define debuglm



//     https://maximeborges.github.io/esp-stacktrace-decoder/


#define kondygnacja 0
const uint8_t subpomieszczenie = 10;
//#define newSensorT1 //dodatkowy sesnor
const String me_lokalizacja = "COWoda"+String(kondygnacja);//+"_mqqt_MARM";
#define ATOMIC_FS_UPDATE
#define MFG "MARM.pl Sp. z o.o."
#define wwwport 80
#define PL_lang
#define ENABLE_INFLUX        //if defined sending to influx database is performed at time when mqtt messages is send
#define enableMQTT
#define enableWebSerial //webserial interface
#define enableArduinoOTA
//#define wdtreset
//#define enableMESHNETWORK
//#define enableWifiManager
#define avSensorReadTime 2*60*1000 //sredni czas odczytu czujnikow bazowo 18b20 jest na sztywno


//#boiler_gas_conversion_to_m3  1

#include "sensivity-config-data.h" //it have definitions of sensivity data
#include "config-translate.h" //definitions polish/english translate
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

#define InitTemp -255
#define DS18B20nodata 255
#define DS18B20nodata2 85
#define DS18B20nodata3 -127
#define DS18B20nodata4 -85
#define maxsensors 7            //maksymalna liczba czujnikow w tabeli
#define namelength 15 //ilosc znakow z nazwy czunika

// Your WiFi credentials.
// Set password to "" for open networks.




#define sensitive_size 32
#define sensitive_sizeS "32"


// pins configuration
#define buzzer D0
#define scl_pin D1 //(int sda, int scl);
#define sda_pin D2 // (int sda, int scl);
#define disCLK D3            //pin clk display
#define disDATA D4           //pin data display  d1, d4 led d7 d8
#define relay1 D5         //pin relay1 WODA
#define relay2 D6         //pin relay2 -drugi przekaznik dla panic CO
#define ONE_WIRE_BUS D7
#define wificonfig_pin D8 //D8 // D3 do wyzwolenia config
#define gas_ain A0
#define ads_0 0
#define ads_1 1
#define ads_2 2
#define ads_3 3
//*************************************************
//#define update_username "admin"
//#define update_password "admin"
#define slashstr "/"
#define update_path "/update"
#define gas_leakage 250 //po tej wartosci alarm
#define histereza_def 1     //po przekroczeniu o tyle stopni temp co w stosunku do wody włącz pompe
#define forceCObelow_def 12        //wymusza pompe CO poniżej temperatury średniej zewnetrznej
#define coConstTempCutOff_def 28  //Temperatura graniczna na wymienniku oznacza ze piec sie grzeje
#define last_case 6 //ilosc wyswietlen na display
#define saveminut (15 * 60 * 1000) //how often in minutes save energy in LittleFS

//#define sensor_18b20_numer 7
#define dbmtemperature "BRoom_Temperature"
#define dbmpressure  "BRoom_Pressure"
#define  dbmhigh  "BRoom_Attit"
#define  dbmhighr  "BRoom_AttitR"
#define  dcoS  "CO_ppm"
#define  dpump1  "pump1WO"   //woda
#define pumpWaterRelay 1
#define  dpump2 "pump2CO"  //CO
#define pumpCoRelay 2
#define  dcoThermstat  "co_Thermstat"
#define uptimelink "uptime"
#define jsonlink "jsonlink"
#define dwaterThermstat  "water_Thermstat"
#define dNThermometerS  "N_Thermometer"
#define dWThermometerS  "W_Thermometer"
#define dEThermometerS  "E_Thermometer"
#define dSThermometerS "S_Thermometer"
#define dallThermometerS "NEWS"
#define dpump1energyS "pump1energyWO"
#define dpump2energyS "pump2energyCO"
#define do_stopkawebsiteS "do_stopkawebsiteS"
#define sensnamelen 32 //dlugosc nazwy czujnika temp 18b20
#define gain_resolution GAIN_ONE

#define DallSens1_addr "28ff6872801402C1"   //coTherm
#define DallSens2_addr "28ff9C6b801402d3"   //waterTherm
#define DallSens3_addr "28FFB66980140291"   //NTherm
#define DallSens4_addr "28FF78668014028B"   //WTherm
#define DallSens5_addr "28ffc6258014020e"   //ETherm
#define DallSens6_addr "28FF4B7280140277"   //STherm
#define DallSens1_name "coThermometer"
#define DallSens2_name "waterThermometer"


bool forceCO = false,
     forceWater = false,
     isadslinitialised = false,
     shouldSaveConfig = false,
     initialConfig = false,
     firstrun = true,  //by szybciej display pokazal temps
     panicbuz = false,
     C_W_state = LOW,
     last_C_W_state = LOW,
     prgstatusrelay1WO = HIGH,  //do kontroli przekaznika by je wylaczyc gdy byly wymuszone [przelacznikiem
     prgstatusrelay2CO = HIGH,  //do kontroli przekaznika by je wylaczyc gdy byly wymuszone [przelacznikiem
     ExistBM280 = false,
     najpierwCO = false;  //dla przelaczenia priorytetu najpierw grzanie wody czy co bez forceCO/Water

char WSSID[31],
     WPass[51],
    //  coThermometer[]      = "28ff6872801402C1",
    //  waterThermometer[]   = "28ff9C6b801402d3",
    //  sens2[]="28FFB66980140291",
    //  sens3[]="28FF78668014028B",
    //  sens4[]="28ffc6258014020e",
    //  sens5[]="28FF4B7280140277",
     ssid[sensitive_size] = SSID_Name,
     pass[sensitive_size] = SSID_PAssword,
     mqtt_server[sensitive_size*2] = MQTT_servername,   // Your MQTT broker address and credentials
     mqtt_user[sensitive_size] = MQTT_username,
     mqtt_password[sensitive_size] = MQTT_Password_data;



const double pumpmincurrent = 0.0005; //minimalna wartosc pradu dla okreslenia czy pompa chodzi 0,0005 means 0,1W

double energy1used,   //used energy
       energy2used,
       allEnergy,
       savetime,
       checkpumptime, //helper to count time to save energy
       commonVolt = 230.00,
       commonFreq = 50.00,
       bmTemp = 0,
       coTherm = InitTemp,
       waterTherm = InitTemp,
       NTherm = InitTemp,
       ETherm = InitTemp,
       WTherm = InitTemp,
       STherm = InitTemp,
       OutsideTempAvg = InitTemp,
       histereza = histereza_def,
       forceCObelow = forceCObelow_def,
       coConstTempCutOff = coConstTempCutOff_def,
       pump1energyLast = 0,
       pump2energyLast = 0,
       dcoval,                // CO sensor valuer
       waitCOStartingmargin; //dla opoznienia przez 1 godzine wymuszenia pompy co gdy nizsza temp CO niz startowa.

const float panic = 70.0; //temperatura dla paniki -przekroczenia na piecu

float bm_high_real,
      bm_high,
      multiplier = 0.125F;      //zastosowany r pomiarowy 10k -ogranicza to pomiart pradu do max 0,7A? tj ok. 200W
                          // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
                          // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
                          // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
                          // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
                          // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
                          // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

int what_display = 0, //dla rotacji wyswietlacza
    mqtt_offline_retrycount = 0,
    mqtt_offline_retries = 10, // retries to mqttconnect before timewait
    count_nowifi=0,
    counter =0,             //to reduce runtime in loop
    mqtt_port = MQTT_port_No;

int32_t  dbmpressval;

unsigned int runNumber = 0, // count of restarts
             publishhomeassistantconfig = 4,                               // licznik wykonan petli -strat od 0
             publishhomeassistantconfigdivider = 5;                        // publishhomeassistantconfig % publishhomeassistantconfigdivider -publikacja gdy reszta z dzielenia =0 czyli co te ilosc wykonan petli opoznionej update jest wysylany config

long C_W_delay = 8000;      // config delay 10 seconds


const unsigned long wifi_checkDelay = 30000,
                    mqtt_offline_reconnect_after_ms = 15 * 60 * 1000,      // time when mqtt is offline to wait for next reconnect (15minutes)

                    ReadTimeTemps = 90 * 1000,      //Interval TempsUpdate
                    ReadEnergyTimeInterval = 1 * 1000;

unsigned long wifimilis,
              time_last_C_W_change = 0,
              lastmqtt_reconnect = 0,



              lastReadTimeTemps = 0,
              lastReadTime1 = 0,                //time last energy1 read
              lastReadTime2 = 0,                //time last energy2 read
              lastEnergyRead = 0,
              lastCOReadTime;


String ownother, UnassignedTempSensor;


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

const int mqtt_Retain = 1;
const String tempicon="<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
const String presicon="<i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
const String attiicon="<i class=\"fas fa-water\" style=\"color:#90add6;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
const String ppmicon="<i class=\"fas fa-tint\" style=\"color:#50ad00;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
const String pumpicon="<i class=\"fas fa-tint\" style=\"color:#500d00;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";
const String humidicon="<i class=\"fas fa-humidity\" style=\"color:blue;font-size:36px;text-shadow:2px 2px 4px #000000;\"></i>&nbsp;&nbsp;";

const String mqttQOS = "0";
const String mqttident = "CO"+String(kondygnacja)+"_";





#include <EEPROM.h>

#define CONFIG_VERSION "V01" sensitive_sizeS

// Where in EEPROM?
#define CONFIG_START 1+sizeof(runNumber)+sizeof(energy1used)*2+16

typedef struct
{
  char version[6]; // place to detect if settings actually are written
  char ssid[sensitive_size];
  char pass[sensitive_size];
  char mqtt_server[sensitive_size*2];
  char mqtt_user[sensitive_size];
  char mqtt_password[sensitive_size];
  int mqtt_port;
  bool najpierwCO;
  bool forceWater;
  bool forceCO;
  double histereza;
  double forceCObelow;
  double coConstTempCutOff;
  // float roomtempset1;
  // float roomtempset2;

  // //float roomtemp;        //now is static sensor so for while save last value
  // float temp_NEWS;
  // char COPUMP_GET_TOPIC[255];  //temperatura outside avg NEWS
  // char NEWS_GET_TOPIC[255];   //pompa CO status
  // char BOILER_FLAME_STATUS_TOPIC[255];   //pompa CO status for 1st temp room sensor
  // char BOILER_FLAME_STATUS_ATTRIBUTE[255];   //pompa CO status for 2nd temp room sensor
  // char BOILER_COPUMP_STATUS_ATTRIBUTE[255];
} configuration_type;

// with DEFAULT values!
configuration_type CONFIGURATION;
configuration_type CONFTMP;

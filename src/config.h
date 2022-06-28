/*server.
  Topic structure is built with get/set mechanizm for
  compatibility with home assistant and to allow external
  device control
  _GET_ topics are used to publish current thermostat state
  _SET_ topics are used to control the thermostat

  Works also offline.

*/

// v.2.0 Initial after move from Arduino Mega to ESP32

//#define debug		//Serial Debug
//#define debug1

#define enableArduinoOTA
//#define wdtreset

//     https://maximeborges.github.io/esp-stacktrace-decoder/


#define kondygnacja 0
const uint8_t subpomieszczenie = 10;
//#define newSensorT1 //dodatkowy sesnor
const String me_lokalizacja = "COWoda"+String(kondygnacja);//+"_mqqt_MARM";
#define ATOMIC_FS_UPDATE
#define MFG "MARM.pl Sp. z o.o."
#define wwwport 80
#define PL_lang
#define  ENABLE_INFLUX        //if defined sending to influx database is performed at time when mqtt messages is send
#define enableWebSerial //webserial interface
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
     sendlogtomqtt = false,
     receivedmqttdata = false,
     isadslinitialised = false,
     shouldSaveConfig = false,
     initialConfig = false,
     starting = true,
     firstrun = true,  //by szybciej display pokazal temps
     panicbuz = false,
     C_W_state = LOW,
     last_C_W_state = LOW,
     prgstatusrelay1WO = HIGH,  //do kontroli przekaznika by je wylaczyc gdy byly wymuszone [przelacznikiem
     prgstatusrelay2CO = HIGH,  //do kontroli przekaznika by je wylaczyc gdy byly wymuszone [przelacznikiem
     ExistBM280 = false,
     najpierwCO = false,  //dla przelaczenia priorytetu najpierw grzanie wody czy co bez forceCO/Water
     firstConnectSinceBoot = false;

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

char log_chars[256];

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
    mqttReconnects = 0,
    mqtt_port = MQTT_port_No;

int32_t  dbmpressval;
#define lampPin LED_BUILTIN

unsigned int runNumber = 0, // count of restarts
             publishhomeassistantconfig = 4,                               // licznik wykonan petli -strat od 0
             publishhomeassistantconfigdivider = 5;                        // publishhomeassistantconfig % publishhomeassistantconfigdivider -publikacja gdy reszta z dzielenia =0 czyli co te ilosc wykonan petli opoznionej update jest wysylany config

long C_W_delay = 8000,      // config delay 10 seconds
     uptime=0;              //tymczasowo

const unsigned long wifi_checkDelay = 30000,
                    mqttUpdateInterval_ms = 1 * 60 * 1000,      //send data to mqtt and influxdb
                    mqtt_offline_reconnect_after_ms = 15 * 60 * 1000,      // time when mqtt is offline to wait for next reconnect (15minutes)
                    WIFIRETRYTIMER = 25 * 1000,
                    ReadTimeTemps = 90 * 1000,      //Interval TempsUpdate
                    ReadEnergyTimeInterval = 1 * 1000;

unsigned long wifimilis,
              time_last_C_W_change = 0,
              lastmqtt_reconnect = 0,
              LOOP_WAITTIME = 30*1000,    //for loop
              lastloopRunTime = 0,        //for loop
              lastWifiRetryTimer = 0,
              lastUpdatemqtt = 0,
              lastReadTimeTemps = 0,
              lastReadTime1 = 0,                //time last energy1 read
              lastReadTime2 = 0,                //time last energy2 read
              lastEnergyRead = 0,
              lastCOReadTime;

size_t content_len;
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
const String mqttdeviceid = "\"dev\":{\"ids\":\""+String(me_lokalizacja)+"\",\"name\":\""+String(me_lokalizacja)+"\",\"sw\":\"" + String(me_version) + "\",\"mdl\": \""+String(me_lokalizacja)+"\",\"mf\":\"" + String(MFG) + "\"}";
const String BASE_TOPIC = me_lokalizacja;
const String BASE_HA_TOPIC = "homeassistant";
const String OUTSIDE = "outside";
const String BOILERROOM = "boilerroom";
const String TEMPERATURE = "_temperature";

const String LOG_TOPIC = BASE_TOPIC + "/log";
const String WILL_TOPIC = BASE_TOPIC + "/Will";
const String IP_TOPIC = BASE_TOPIC + "/IP";
const String STATS_TOPIC = BASE_TOPIC + "/stats";

const String SET_LAST = "/set";
const String SENSOR = "/sensor/";
const String SWITCH = "/switch/";

const String OUTSIDE_TEMPERATURE_N = OUTSIDE + TEMPERATURE + "_North";
const String OUTSIDE_TEMPERATURE_E = OUTSIDE + TEMPERATURE + "_East";
const String OUTSIDE_TEMPERATURE_W = OUTSIDE + TEMPERATURE + "_West";
const String OUTSIDE_TEMPERATURE_S = OUTSIDE + TEMPERATURE + "_South";
const String OUTSIDE_TEMPERATURE_A = OUTSIDE + TEMPERATURE + "_Averange";
#ifdef newSensorT1
const String BOILERROOM_TEMPERATURE_T1 = BOILERROOM + TEMPERATURE + "_Spare";
#endif
const String HEATERCO_TEMPERATURE = BOILERROOM + TEMPERATURE + "_CO";
const String WATER_TEMPERATURE = BOILERROOM + TEMPERATURE + "_Water";
const String BOILERROOM_TEMPERATURE = BOILERROOM + TEMPERATURE;
const String BOILERROOM_PRESSURE = BOILERROOM + "_pressure";
const String BOILERROOM_HIGH = BOILERROOM + "_high";
const String BOILERROOM_HIGHREAL = BOILERROOM + "_highreal";
const String BOILERROOM_COVAL = BOILERROOM + "_gasCO";
const String BOILERROOM_PUMP1WA = BOILERROOM + "_pump1Water";
const String BOILERROOM_PUMP1WA_E = BOILERROOM + "_pump1Water_Energy";
const String BOILERROOM_PUMP2CO = BOILERROOM + "_pump2CO";
const String BOILERROOM_PUMP2CO_E = BOILERROOM + "_pump2CO_Energy";

const String BOILERROOM_SWITCH_TOPIC = BASE_TOPIC + SWITCH + BOILERROOM + "/attributes";
const String BOILERROOM_SWITCH_TOPIC_SET = BASE_TOPIC + SWITCH + BOILERROOM + "/set";
const String BOILERROOM_HA_SWITCH_TOPIC = BASE_HA_TOPIC + SWITCH + BASE_TOPIC + "/" + BOILERROOM;     //+"/state"
const String BOILERROOM_SENSOR_TOPIC = BASE_TOPIC + SENSOR + BOILERROOM + "/attributes";
const String BOILERROOM_HA_SENSOR_TOPIC = BASE_HA_TOPIC + SENSOR + BASE_TOPIC + "/" + BOILERROOM;     //+"/state"
//Subscribe
String SUPLA_VOLT_TOPIC = "electricmain/supla/devices/zamel-mew-01-99a200/channels/0/state/phases/3/voltage";
String SUPLA_FREQ_TOPIC = "electricmain/supla/devices/zamel-mew-01-99a200/channels/0/state/phases/3/frequency";


// const String ROOM_TEMPERATURE = ROOM_TEMP + TEMPERATURE;
// const String ROOM_TEMPERATURE_SETPOINT = ROOM_TEMPERATURE + "_setpoint";
// const String ROOM_TEMPERATURE_SETPOINT_SET_TOPIC = BASE_TOPIC + "/SET/" + ROOM_TEMPERATURE_SETPOINT; //+SET_LAST + "/set"; // t


// const String ROOMS_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + ROOM_TEMP; //+"/state"





// const String BOILER = "boiler";
// const String HOT_WATER = "domestic_hot_water";
// const String ROOM_OTHERS = "room_other";

// const String BOILER_TEMPERATURE = BOILER + TEMPERATURE;
// // const String BOILER_MOD = BOILER+"-mode";   //tryb pracy
// const String BOILER_TEMPERATURE_RET = BOILER + TEMPERATURE + "_return";
// const String BOILER_TEMPERATURE_SETPOINT = BOILER + TEMPERATURE + "_setpoint";
// // const String BOILER_CH_STATE = BOILER + "_ch_state";
// const String BOILER_SOFTWARE_CH_STATE_MODE = BOILER + "_software_ch_state_and_mode";
// // const String FLAME_STATE = "flame_state";
// // const String FLAME_LEVEL = "flame_level";
// const String TEMP_CUTOFF = "temp_cutoff";
// // const String FLAME_W = "flame_used_energy";
// // const String FLAME_W_TOTAL = "flame_used_energy_total";

// // const String HOT_WATER_TEMPERATURE = HOT_WATER + TEMPERATURE;
// const String HOT_WATER_TEMPERATURE_SETPOINT = HOT_WATER + TEMPERATURE + "_setpoint";
// // const String HOT_WATER_CH_STATE = HOT_WATER + "_dhw_state";
// const String HOT_WATER_SOFTWARE_CH_STATE = HOT_WATER + "_software_dhw_state";


// // const String ROOM_OTHERS_PRESSURE = ROOM_OTHERS + "_pressure";

// const String BOILER_TOPIC = BASE_TOPIC + "/" + BOILER + "/attributes";
// // const String HOT_WATER_TOPIC = BASE_TOPIC + "/" + HOT_WATER + "/attributes";



// const String TEMP_SETPOINT_SET_TOPIC = BASE_TOPIC + "/SET/" + BOILER_TEMPERATURE_SETPOINT + "/set";  // sp
// const String TEMP_CUTOFF_SET_TOPIC = BASE_TOPIC + "/SET/" + TEMP_CUTOFF + "/set";                    // cutOffTemp
// const String STATE_DHW_SET_TOPIC = BASE_TOPIC + "/SET/" + HOT_WATER_SOFTWARE_CH_STATE + "/set";      // enableHotWater
// const String MODE_SET_TOPIC = BASE_TOPIC + "/SET/" + BOILER_SOFTWARE_CH_STATE_MODE + "/set";         // 012 auto, heat, off ch
// const String TEMP_DHW_SET_TOPIC = BASE_TOPIC + "/SET/" + HOT_WATER_TEMPERATURE_SETPOINT + "/set";    // dhwTarget
// String COPUMP_GET_TOPIC = "COWoda_mqqt_MARM/switch/bcddc2b2c08e/pump2CO/state";                      // temperatura outside avg NEWS
// String NEWS_GET_TOPIC = "COWoda_mqqt_MARM/sensor/bcddc2b2c08e/WENS_Outside_Temp_AVG/state";          // pompa CO status
// String BOILER_FLAME_STATUS_TOPIC = "opentherm-thermostat/boiler/attributes";                              //flme status of co gaz boiler
// String BOILER_FLAME_STATUS_ATTRIBUTE = "ot_flame_state";                              //boiler flame status of co gaz boiler
// String BOILER_COPUMP_STATUS_ATTRIBUTE = "ot_boiler_ch_state";                          //boiler pump status

// // logs topic
// const String DIAGS = "diag";
// const String DIAG_TOPIC = BASE_TOPIC + "/" + DIAGS + "/attributes";
// const String DIAG_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";
// const String DIAG_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";

// const String LOGS = "log";
// const String LOG_GET_TOPIC = BASE_TOPIC + "/" + DIAGS + "/" + LOGS;
// const String INTEGRAL_ERROR_GET_TOPIC = DIAGS + "_" + "interr";
// const String DIAGS_OTHERS_FAULT = DIAGS + "_" + "fault";
// const String DIAGS_OTHERS_DIAG = DIAGS + "_" + "diagnostic";

// //Homeassistant Autodiscovery topics
// const String BOILER_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";              //+"/state"
// const String BOILER_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";     //+"/state"
// const String BOILER_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + BOILER; //+"/state"

// const String HOT_WATER_HA_TOPIC = BASE_HA_TOPIC + "/sensor/" + BASE_TOPIC + "/";                 //+"/state"
// const String HOT_WATER_HABS_TOPIC = BASE_HA_TOPIC + "/binary_sensor/" + BASE_TOPIC + "/";        //+"/state"
// const String HOT_WATER_HACLI_TOPIC = BASE_HA_TOPIC + "/climate/" + BASE_TOPIC + "/" + HOT_WATER; //+"/state"





// // setpoint topic
// const String SETPOINT_OVERRIDE = "setpoint-override";
// const String SETPOINT_OVERRIDE_SET_TOPIC = BASE_TOPIC + "/" + SETPOINT_OVERRIDE + "/set";     // op_override
// const String SETPOINT_OVERRIDE_RESET_TOPIC = BASE_TOPIC + "/" + SETPOINT_OVERRIDE + "/reset"; //





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

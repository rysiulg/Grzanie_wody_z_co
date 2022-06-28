// extern "C"
// {
// #include "user_interface.h"
// }
#include <Arduino.h>

//#include <Update.h>

const char version[12+1] =
{
   // YY year
   __DATE__[9], __DATE__[10],

   // First month letter, Oct Nov Dec = '1' otherwise '0'
   (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? '1' : '0',

   // Second month letter
   (__DATE__[0] == 'J') ? ( (__DATE__[1] == 'a') ? '1' :       // Jan, Jun or Jul
                            ((__DATE__[2] == 'n') ? '6' : '7') ) :
   (__DATE__[0] == 'F') ? '2' :                                // Feb
   (__DATE__[0] == 'M') ? (__DATE__[2] == 'r') ? '3' : '5' :   // Mar or May
   (__DATE__[0] == 'A') ? (__DATE__[1] == 'p') ? '4' : '8' :   // Apr or Aug
   (__DATE__[0] == 'S') ? '9' :                                // Sep
   (__DATE__[0] == 'O') ? '0' :                                // Oct
   (__DATE__[0] == 'N') ? '1' :                                // Nov
   (__DATE__[0] == 'D') ? '2' :                                // Dec
   0,

   // First day letter, replace space with digit
   __DATE__[4]==' ' ? '0' : __DATE__[4],

   // Second day letter
   __DATE__[5],
   __TIME__[0],__TIME__[1],
   __TIME__[3],__TIME__[4],
  '\0'
};
const String me_version = String(version);
#include "config.h"
const String  stopka = String(MFG)+" "+version[4]+version[5]+"-"+version[2]+version[3]+"-20"+version[0]+version[1]+" "+version[6]+version[7]+":"+version[8]+version[9];



// struct tempsensor {
//   uint8_t addressHEX[8];
//   uint8_t addressHEXnew[8];
//   float tempread;                 //actual temperature
//   float tempset;                  //temperature set
//   float humidityread;             //actual humidity
//   char nameSensor[namelength];
//   int idpinout;                    //numer portu digital wyjscia sterowania.
//   bool switch_state;               //is pump or other device active
// };

// tempsensor TempSensor[maxsensors];



//#include <FS.h>       // ---- esp board manager 2.7.1 --- iwip Variant V2
//#include <LittleFS.h>
//#include <Ethernet.h>


#include "SSTM1637.h"  //#include <SevenSegmentTM1637.h>

//#include <ESPAsyncWiFiManager.h>

//#include <ArduinoJson.h> //--------- https://github.com/bblanchon/ArduinoJson/tree/v5.13.2 ------
//#include <EEPROM.h>


#ifdef ESP32
//#include <WiFi.h>
//#include <Update.h>
#include <AsyncTCP.h>
#include <AsyncUDP.h>
//#include <ESPmDNS.h>
#include <AsyncDNSServer.h>
#include "esp_task_wdt.h"
#else
#include <Updater.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
//#include <ESPAsyncUDP.h>
//#include <ESPAsyncDNSServer.h>
#endif

#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>

#ifdef enableWebSerial
#include <WebSerial.h>
#endif
#ifdef ENABLE_INFLUX
  #ifdef ESP32
  #include <HTTPClient.h>
  #include <InfluxDbClient.h>
  #else
  #include <ESP8266HTTPClient.h>
  #include <InfluxDbClient.h>
  #endif
#endif

#ifdef doubleResDet
#include <ESP_DoubleResetDetector.h>
#endif

#ifdef enableWifiManager
#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager
#endif
#include <DNSServer.h>
#include <String.h>

#if !( defined(ESP8266) )
  #error This code is intended to run on ESP8266 platform! Please check your Tools->Board setting.
#endif

//#include <AsyncElegantOTA.h>

#include <DallasTemperature.h>
#include <OneWire.h>
#include "BMP085.h"
// #include <SPI.h>
#include <Adafruit_ADS1X15.h>
#ifdef enableArduinoOTA
#include <ArduinoOTA.h>
#endif
#include <SPI.h>

AsyncWebServer webserver(wwwport);
Adafruit_BMP085 bmp; //0x77 addr
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Adafruit_ADS1X15 ads1;//(0x48); //na wszelki wypadek zrobie jako drugi adres zamiast 48
SevenSegmentTM1637 display(disCLK, disDATA);
//TM1637Display    display(disCLK, disDATA);
WiFiClient espClient;
PubSubClient mqttclient(espClient);

#ifdef ENABLE_INFLUX
InfluxDBClient InfluxClient(INFLUXDB_URL, INFLUXDB_DB_NAME);
Point InfluxSensor(InfluxMeasurments);
#endif
#ifdef doubleResDet
  #define DRD_TIMEOUT 0.1

  // address to the block in the RTC user memory
  // change it if it collides with another usageb
  // of the address block
  #define DRD_ADDRESS 0x00
  DoubleResetDetector* drd;
#endif
DNSServer dnsServer;



uint8_t mac[6] = {(uint8_t)strtol(WiFi.macAddress().substring(0,2).c_str(),0,16), (uint8_t)strtol(WiFi.macAddress().substring(3,5).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(6,8).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(9,11).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(12,14).c_str(),0,16),(uint8_t)strtol(WiFi.macAddress().substring(15,17).c_str(),0,16)};



//GAS:
/************************Hardware Related Macros************************************/
#define         RL_VALUE                     (10)    //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR          (9.21)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                     //which is derived from the chart in datasheet
/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (10)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (0)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (0)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (1)     //define the time interal(in milisecond) between each samples in
                                                     //normal operation
/**********************Application Related Macros**********************************/
#define         GAS_H2                      (0)
/*****************************Globals***********************************************/
float           H2Curve[3]  =  {2.3, 0.93,-1.44};    //two points are taken from the curve in datasheet.
                                                     //with these two points, a line is formed which is "approximately equivalent"
                                                     //to the original curve.
                                                     //data format:{ x, y, slope}; point1: (lg200, lg8.5), point2: (lg10000, lg0.03)
float           Ro           =  10; //18.07; //po kalibracji 10;                  //Ro is initialized to 10 kilo ohms
//ENDGAS
  //Stężenie: Oznaki i objawy
  //~ 100 ppm Lekki ból głowy, wypieki (nieokreślony czas narażenia)
  //200–300 ppm Ból głowy (czas narażenia 5–6 godz.)
  //400–600 ppm Silny ból głowy, osłabienie, zawroty głowy, nudności, wymioty (czas narażenia 4–5 godz.)
  //1,100–1,500 ppm Przyspieszone tętno i oddech, omdlenie (zasłabnięcie), śpiączka, przerywane ataki drgawek (czas narażenia 4–5 godz.)
  //5,000–10,000 ppm Słabe tętno, płytki oddech/zatrzymanie oddychania, śmierć (czas narażenia 1–2 minuty)
  //Źródło: Brytyjska Agencja Ochrony Zdrowia (HPA), Kompendium zagrożeń chemicznych, Tlenek węgla, Wersja 3, 2011
  //
  //Detektor tlenku węgla uaktywni się, jeśli wykryje określone stężenie gazu w określonym przedziale czasu zgodnie z poniższym opisem:
  //50 ppm: Alarm w ciągu 60–90 minut
  //100 ppm: Alarm w ciągu 10–40 minut
  //300 ppm: Alarm w ciągu 3 minut
  //Czujnik czadu został ustawiony zgodnie z odpowiednią normą europejską, tak aby alarmy były wyzwalane w zależności od stężenia gazu i okresu jego występowania. Alarm uaktywnia się, gdy poziom stężenia CO narasta w czasie do niebezpiecznych poziomów lub wymagane jest natychmiastowe działanie. Natomiast unikane są fałszywe alarmy związane z tymczasowymi niskimi stężeniami CO (np. w wyniku działania dymu papierosowego).




//common_functions.h
void log_message(char* string);
String uptimedana(unsigned long started_local);
String getJsonVal(String json, String tofind);
bool isValidNumber(String str);
String convertPayloadToStr(byte *payload, unsigned int length);
int dBmToQuality(int dBm);
int getWifiQuality();
int getFreeMemory();
bool PayloadStatus(String payloadStr, bool state);
bool PayloadtoValidFloatCheck(String payloadStr);
float PayloadtoValidFloat(String payloadStr,bool withtemps_minmax=false, float mintemp=InitTemp, float maxtemp=InitTemp);
void restart();
String getIdentyfikator(int x);
#ifdef enableArduinoOTA
void setupOTA();
#endif
char* dtoa(double dN, char *cMJA, int iP);


void setup();
void getCOGAS(int pinpriv);
double getkWh(double EnergyAmpHourValue);
void ReadEnergyUsed();
void ReadTemperatures();
void displayCoCo();
void loop();


char* dtoa(double dN, char *cMJA, int iP);
void onMqttMessage(const char* topic, const uint8_t* payload, uint16_t length);
void onMqttConnected();
void onMqttConnectionFailed();

void ReadConfigEnergy();  //odczytaj z spiffs konfiguracje i zuzyta energie -setup
void SaveEnergy();        //zapisz do spiff stan zuzycia energii
void SaveConfig();        //zapisz w spiffs configuracje
void gas_leak_check();    //sprawdz czujnik gazu
void display_temp_rotation();   //obsluga rotacji wyswietlacza
void starthttpserver();           //obsluga serwerow www
void notFound(AsyncWebServerRequest *request);
String PrintHex8(const uint8_t *data, char separator, uint8_t length); // prints 8-bit data in hex , uint8_t length
String checkUnassignedSensors();
#ifdef enableWifiManager
void ondemandwifiCallback ();
#endif
void saveConfigCallback ();
void hexCharacterStringToBytes(byte *byteArray, const char *hexString);
byte nibble(char c);
void dumpByteArray(const byte * byteArray);
void array_to_string(byte array[], unsigned int len, char buffer[]);
void handleUpdate(AsyncWebServerRequest *request);
void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
void printProgress(size_t prg, size_t sz);
void ChangeRelayStatus(uint8_t numer, bool stan);
void check_temps_pumps();
String getlinki();
int  MQGetPercentage(float rs_ro_ratio, float *pcurve);
int MQGetGasPercentage(float rs_ro_ratio, int gas_id);
float MQRead(int mq_pin);
float MQCalibration(int mq_pin);
float MQResistanceCalculation(int raw_adc);
double getenergy(int adspin);

String getJsonVal(String json, String tofind);
void ReadTemperatures();
String convertPayloadToStr(byte *payload, unsigned int length);
void mqttCallback(char *topic, byte *payload, unsigned int length);
void mqtt_reconnect();
#ifdef ENABLE_INFLUX
void updateInfluxDB();
#endif
void updateMQTTData();
void mqttHAPublish_Config (String HADiscoveryTopic, String ValueTopicName, String SensorName, String friendlySensorName, int unitClass, String cmd_temp = String('\0'));
#ifdef enableWebSerial
void recvMsg(uint8_t *data, size_t len);
#endif
String do_stopkawebsite();
void restart();
bool PayloadStatus(String payloadStr, bool state);
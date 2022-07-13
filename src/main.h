// extern "C"
// {
// #include "user_interface.h"
// }
#include <Arduino.h>

//#include <Update.h>


#include "config.h"



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

#include <ESPAsyncWebServer.h>


#ifdef enableWifiManager
#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager
#endif

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

#include <SPI.h>


Adafruit_BMP085 bmp; //0x77 addr
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Adafruit_ADS1X15 ads1;//(0x48); //na wszelki wypadek zrobie jako drugi adres zamiast 48
SevenSegmentTM1637 display(disCLK, disDATA);
//TM1637Display    display(disCLK, disDATA);




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
void mqtt_callback(char *topic, byte *payload, unsigned int length);
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
double getkWh(double EnergyAmpHourValue);
void getCOGAS(int pinpriv = gas_ain);
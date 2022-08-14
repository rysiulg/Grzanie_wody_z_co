// extern "C"
// {
// #include "user_interface.h"
// }
#include <Arduino.h>
#include "Common_symlinkFiles\common_functions.h"

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


#include "SSTM1637.h"  //#include <SevenSegmentTM1637.h>

#ifdef enableWifiManager
#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager
#endif

//#include <String.h>

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



String SaveEnergy(int EpromPosition);        //zapisz do spiff stan zuzycia energii
void gas_leak_check();    //sprawdz czujnik gazu
void display_temp_rotation();   //obsluga rotacji wyswietlacza
void starthttpserver();           //obsluga serwerow www
void notFound(AsyncWebServerRequest *request);
String PrintHex8(const uint8_t *data, char separator, uint8_t length); // prints 8-bit data in hex , uint8_t length
String checkUnassignedSensors();
#ifdef enableWifiManager
void ondemandwifiCallback ();
#endif
// void saveConfigCallback ();

void ChangeRelayStatus(uint8_t numer, bool stan);
void check_temps_pumps();
String getlinki();
int  MQGetPercentage(float rs_ro_ratio, float *pcurve);
int MQGetGasPercentage(float rs_ro_ratio, int gas_id);
float MQRead(int mq_pin);
float MQCalibration(int mq_pin);
float MQResistanceCalculation(int raw_adc);
double getenergy(int adspin);

void ReadTemperatures();

// void mqtt_callback(char *topic, byte *payload, unsigned int length);
// void mqtt_reconnect();

// void mqttHAPublish_Config (String HADiscoveryTopic, String ValueTopicName, String SensorName, String friendlySensorName, int unitClass, String cmd_temp = String('\0'));

String do_stopkawebsite();

double getkWh(double EnergyAmpHourValue);
void getCOGAS(int pinpriv = gas_ain);

#include "funkcje.h"
#if defined enableMQTT || defined enableMQTTAsync || defined ENABLE_INFLUX
#include "mqtt_influx.h"
#endif
#ifdef enableWifiManager
#include "configPortal.h"
#endif
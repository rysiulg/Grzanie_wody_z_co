const uint8_t subpomieszczenie = 10;
#ifndef tmplok
#define tmplok ""
#endif
String me_lokalizacja = "COWODA"+String(kondygnacja)+ String(tmplok);//+"_mqqt_MARM";

//#define sensor_18b20_numer 7
// #define dbmtemperature "BRoom_Temperature"
// #define dbmpressure  "BRoom_Pressure"
// #define  dbmhigh  "BRoom_Attit"
// #define  dbmhighr  "BRoom_AttitR"
// #define  dcoS  "CO_ppm"
// #define  dpump1  "pump1WO"   //woda
// #define dpump2 "pump2CO"  //CO
// #define dcoThermstat  "co_Thermstat"
// #define uptimelink "uptime"
// #define jsonlink "jsonlink"
// #define dwaterThermstat  "water_Thermstat"
// #define dNThermometerS  "N_Thermometer"
// #define dWThermometerS  "W_Thermometer"
// #define dEThermometerS  "E_Thermometer"
// #define dSThermometerS "S_Thermometer"
// #define dallThermometerS "NEWS"
// #define dpump1energyS "pump1energyWO"
// #define dpump2energyS "pump2energyCO"
// #define do_stopkawebsiteS "do_stopkawebsiteS"
// #define sensnamelen 32 //dlugosc nazwy czujnika temp 18b20
#define gain_resolution GAIN_ONE

// #define DallSens1_addr "28ff6872801402C1"   //coTherm
// #define DallSens2_addr "28ff9C6b801402d3"   //waterTherm
// #define DallSens3_addr "28FFB66980140291"   //NTherm
// #define DallSens4_addr "28FF78668014028B"   //WTherm
// #define DallSens5_addr "28ffc6258014020e"   //ETherm
// #define DallSens6_addr "28FF4B7280140277"   //STherm
// #define DallSens1_name "coThermometer"
// #define DallSens2_name "waterThermometer"




// pins configuration
#define pumpWaterRelay 1  //wather
#define pumpCoRelay 2    //CO
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

#define gas_leakage 250 //po tej wartosci alarm
#define histereza_def 1     //po przekroczeniu o tyle stopni temp co w stosunku do wody włącz pompe
#define forceCObelow_def 12        //wymusza pompe CO poniżej temperatury średniej zewnetrznej
#define coConstTempCutOff_def 28  //Temperatura graniczna na wymienniku oznacza ze piec sie grzeje
#define last_case 6 //ilosc wyswietlen na display
#define saveminut (15 * 60 * 1000) //how often in minutes save energy in LittleFS

char coThermometerAddr[]      = "28ff6872801402C1",
     waterThermometerAddr[]   = "28ff9C6b801402d3",
     NThermAddr[]="28FFB66980140291",
     WThermAddr[]="28FF78668014028B",
     EThermAddr[]="28ffc6258014020e",
     SThermAddr[]="28FF4B7280140277";


String UnassignedTempSensor = "\0";

bool forceCO = false,
     forceWater = false,
     isadslinitialised = false,
     shouldSaveConfig = false,
     prgstatusrelay1WO = HIGH,  //do kontroli przekaznika by je wylaczyc gdy byly wymuszone [przelacznikiem
     najpierwCO = false,  //dla przelaczenia priorytetu najpierw grzanie wody czy co bez forceCO/Water
     prgstatusrelay2CO = HIGH,  //do kontroli przekaznika by je wylaczyc gdy byly wymuszone [przelacznikiem
     panicbuz = false,
     C_W_state = LOW,
     last_C_W_state = LOW,
     ExistBM280 = false;

const double pumpmincurrent = 0.0005; //minimalna wartosc pradu dla okreslenia czy pompa chodzi 0,0005 means 0,1W
double pump1energyLast = 0,
       pump2energyLast = 0,
       commonVolt = 230.00,
       commonFreq = 50.00,
       bmTemp = 0,
       coTherm = InitTempst,
       waterTherm = InitTempst,
       NTherm = InitTempst,
       ETherm = InitTempst,
       WTherm = InitTempst,
       STherm = InitTempst,
       OutsideTempAvg = InitTempst,
       dcoval,                // CO sensor valuer
       waitCOStartingmargin, //dla opoznienia przez 1 godzine wymuszenia pompy co gdy nizsza temp CO niz startowa.
       histereza = histereza_def,
       forceCObelow = forceCObelow_def,
       coConstTempCutOff = coConstTempCutOff_def,
       energy1used,   //used energy
       energy2used,
       savetime,      //moment of saved time interval
       allEnergy;

const unsigned long ReadEnergyTimeInterval = 1 * 1000,
                    ReadTimeTemps = 90 * 1000;      //Interval TempsUpdate

unsigned long  C_W_delay = 8000,      // config delay 10 seconds
               time_last_C_W_change = 0,
               lastReadTime1 = 0,                //time last energy1 read
               lastReadTime2 = 0,                //time last energy2 read
               lastEnergyRead = 0,
               lastCOReadTime = 0,
               lastReadTimeTemps = 0;

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

int what_display = 0; //dla rotacji wyswietlacza
//     mqtt_offline_retrycount = 0,
//     mqtt_offline_retries = 10, // retries to mqttconnect before timewait
    // count_nowifi=0;
    // counter =0,             //to reduce runtime in loop
    // mqtt_port = MQTT_port_No;

int32_t  dbmpressval;


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

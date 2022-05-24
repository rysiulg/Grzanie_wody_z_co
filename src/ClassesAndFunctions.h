//



//TM1637Display    display(disCLK, disDATA);


#include "DS18B20rgw.h"

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



//byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A};

//EthernetClient client;



//Ex.) char cVal[10];  float fVal=((22.0 /7.0)*256)-46.85;
// dtoa(fVal,cVal,4); Serial.println (String(cVal));

  //arguments...
  // float-double value, char array to fill, precision (4 is .xxxx)
  //and... it rounds last digit

char* dtoa(double dN, char *cMJA, int iP) {
  char *ret = cMJA; long lP=1; byte bW=iP;
  while (bW>0) { lP=lP*10;  bW--;  }
  long lL = long(dN); double dD=(dN-double(lL))* double(lP);
  if (dN>=0) { dD=(dD + 0.5);  } else { dD=(dD-0.5); }
  long lR=abs(long(dD));  lL=abs(lL);
  if (lR==lP) { lL=lL+1;  lR=0;  }
  if ((dN<0) & ((lR+lL)>0)) { *cMJA++ = '-';  }
  ltoa(lL, cMJA, 10);
  if (iP>0) { while (*cMJA != '\0') { cMJA++; } *cMJA++ = '.'; lP=10;
  while (iP>1) { if (lR< lP) { *cMJA='0'; cMJA++; } lP=lP*10;  iP--; }
  ltoa(lR, cMJA, 10); }  return ret; }



namespace Sensor {
class OnePhaseElectricityMeter  {
  public:
    OnePhaseElectricityMeter(char *adcsensor) {
      Sensor::OnePhaseElectricityMeter::adcsensor=adcsensor;
      #ifdef debug
        Serial.print(F("initOPEM ok adcsensor...: "));
        Serial.println((String)adcsensor);
      #endif

      if (String(Sensor::OnePhaseElectricityMeter::adcsensor) == "0") {
        if (isnan(energy1used)) energy1used = 0;
        energyused = energy1used;
      }
      if (String(Sensor::OnePhaseElectricityMeter::adcsensor) == "1") {
        if (isnan(energy2used)) energy2used = 0;
        energyused = energy2used;
      }
      #ifdef debug
        Serial.println(F("initOPEM2 ok..."));
      #endif
    }

    void onInit() {
      lasttimelapse = millis();
      #ifdef debug
        Serial.print(F("OPEM init Start...: "));
        Serial.println(String(adcsensor));
      #endif
      lastEnergyAmpValue=getValue();
      #ifdef debug
        Serial.print(F("OPEM onInit ok...: "));
        Serial.print(F(" lastValidValue: "));
        Serial.println(lastEnergyAmpValue);
      #endif
    }

    double getValue() {
      wdt_reset();
      const double kWat = 1000, godzina = 60 * 60 * 1000;
      #ifdef debug
        Serial.print(F("OPEM getval1 Start."));
        Serial.print(adcsensor);
        Serial.print("  int: ");
        Serial.println(atoi(adcsensor));
      #endif
      lastEnergyAmpValue = getenergy(atoi(adcsensor));
      lastReadTime = millis();
      #ifdef debug
        Serial.print(F("OPEM getval lastvalid: "));
        Serial.println(lastEnergyAmpValue);
      #endif
//      setCurrent(faza, lastEnergyAmpValue * 1000 );
//      setVoltage(faza, 235 * 100);
//      setFreq(50 * 100);
//      setPowerActive(faza, 230 * 100 * lastEnergyAmpValue * 1000);
      double timenow = millis();
      double czaspomiarowy = timenow - lasttimelapse;
      //double powerenergy = commonVolt * lastEnergyAmpValue;
      energyused += (commonVolt * 1 * lastEnergyAmpValue *1) /kWat * czaspomiarowy / godzina;
      if (isnan(energyused)) energyused=0;
      if (String(Sensor::OnePhaseElectricityMeter::adcsensor) == "0") {
        if (isnan(energy1used)) energy1used = 0;
        energy1used = energyused; //+= (commonVolt * 1 * lastEnergyAmpValue *1) /kWat * czaspomiarowy / godzina;
      }
      if (String(Sensor::OnePhaseElectricityMeter::adcsensor) == "1") {
        if (isnan(energy2used)) energy2used = 0;
        energy2used = energyused; //+= (commonVolt * 1 * lastEnergyAmpValue *1) /kWat * czaspomiarowy / godzina;
      }
      #ifdef debug
        Serial.print(F("OPEM getval adcsensor no: "));
        Serial.println(adcsensor);
        Serial.print(F("Energy used: "));
        Serial.println(energyused,5);
        Serial.print(F("Energy1 used: "));
        Serial.println(energy1used,5);
        Serial.print(F("Energy2 used: "));
        Serial.println(energy2used,5);
        Serial.println(F("After set energy"));
      #endif
      lasttimelapse = timenow;




//      OnePhaseElectricityMeterHA->setValue(lastValidValue * 1000);
//   //   wynik = OnePhaseElectricityMeterHA(charaddress).publishValue(dtoa(lastEnergyAmpValue * 1000,"",4));
//      OnePhaseElectricityMeterVolHA->setValue(235 * 100);
//  //    wynik = OnePhaseElectricityMeterHA(charaddress).publishValue(dtoa(235 * 100,"",4));
//      OnePhaseElectricityMeterFreHA->setValue(50 * 100);
//  //    wynik =  OnePhaseElectricityMeterHA(charaddress).publishValue(dtoa(50 * 100,"",4));
//      OnePhaseElectricityMeterPowHA->setValue(230 * 100 * lastEnergyAmpValue * 1000);
  //    wynik =  OnePhaseElectricityMeterPowHA(charaddress).publishValue(dtoa(230 * 100 * lastEnergyAmpValuelidValue * 1000,"",4));
//      OnePhaseElectricityMeterTotHA->setValue((230 * 100 * lastEnergyAmpValue * 1000) / kWat * czaspomiarowy / godzina);
//   //  wynik =  OnePhaseElectricityMeterHA(charaddress).publishValue(dtoa((230 * 100 * lastEnergyAmpValue * 1000) / kWat * czaspomiarowy / godzina,"",4));
      return lastEnergyAmpValue;
    }
    String getlast(int what) {
      //en czynna pobrana wyslana z urzadzenia;(
      double powerenergy = commonVolt * lastEnergyAmpValue;
      switch (what) {
        case energy_current:    return String(lastEnergyAmpValue * 1,4); break;
        case energy_energyUsed: return String(energyused,4); break;
        case energy_power:      return String(powerenergy,1); break;
        case energy_voltage:    return String(commonVolt,2); break;
        case energy_freq:       return String(commonFreq,2); break;
      }
      return String(lastEnergyAmpValue * 1,4);
    }
    void iterateAlways() {
    //HASensor DS18B20HA(char *deviceAddress);
      if (lastReadTime + 15000 < millis()) {
        lastEnergyAmpValue = getValue();
        #ifdef debug
          Serial.print("OPEM Iterate get value: ");
          Serial.println(lastEnergyAmpValue);
        #endif

      }

    }
  protected:
    double lastEnergyAmpValue;
    double energyused;
    double lasttimelapse;
    char *adcsensor;
    unsigned long lastReadTime;
};
//*************************************************




class GasGeneralPurposeMeasurementBase { // : public Supla::Sensor::GeneralPurposeMeasurementBase {
  public:
    GasGeneralPurposeMeasurementBase(int pin) {
  //    channel.setType(SUPLA_CHANNELTYPE_DISTANCESENSOR);
  //    channel.setDefault(SUPLA_CHANNELFNC_DEPTHSENSOR);
      pinpriv = pin;
    }

    int getValue() {
     // value = analogRead(pinpriv);            //calibration from http://sandboxelectronics.com/?p=196

      lastvalue = MQGetPercentage(MQRead(pinpriv)/Ro,H2Curve);
      //lastvalue = (pow(10, ( ((log(((float)Ro*(1023-MQRead(pinpriv))/MQRead(pinpriv)) / (((float)Ro*(1023-60)/60) / RO_CLEAN_AIR_FACTOR)) - H2Curve[1]) / H2Curve[2]) + H2Curve[0])));  //mialem odczyty 60 z pomiarow bez funkcji kalibracji -odjalem 25 by wyrownac do 0....
      if (lastvalue > 20000 or lastvalue < 0) lastvalue = 0;
      lastReadTime = millis();
      dcoval = lastvalue;
      return lastvalue;
    }
    void Init() {
      Ro = MQCalibration(pinpriv);
      lastvalue=getValue();
    }
    void iterateAlways() {
      if ((lastReadTime + avSensorReadTime*2) < millis()) {
        lastvalue=getValue(); //channel.setNewValue(getValue());
      }
    }
    int getlast() {
      return lastvalue;
    }

  protected:
    int lastvalue;
    int pinpriv;
    unsigned long lastReadTime;
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
 };

};

///*
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//*/
//
#define TEMPERATURE_NOT_AVAILABLE -275
#define PRESSURE_NOT_AVAILABLE 0
#ifndef _ds18b20_h
#define _ds18b20_h

//#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

//#include "supla-common/log.h"
//#include "supla/sensor/thermometer.h"

namespace HAM {
namespace Sensor {

class OneWireBus {
 public:
  OneWireBus(uint8_t pinNumber)
      : oneWire(pinNumber), pin(pinNumber), nextBus(nullptr), lastReadTime(0) {
    #ifdef debug
      printf("Initializing OneWire bus at pin %d", pinNumber);
    #endif
    sensors.setOneWire(&oneWire);
    sensors.begin();
    if (sensors.isParasitePowerMode()) {
      #ifdef debug
        printf("\nOneWire(pin %d) Parasite power is ON", pinNumber);
      #endif
    } else {
      #ifdef debug
        printf("\nOneWire(pin %d) Parasite power is OFF", pinNumber);
      #endif
    }

    #ifdef debug
      printf("\nOneWire(pin %d) Found %d devices:",pinNumber,sensors.getDeviceCount());
      Serial.println("");
    #endif

    // report parasite power requirements

    DeviceAddress address;
    char strAddr[64];
    for (int i = 0; i < sensors.getDeviceCount(); i++) {
      if (!sensors.getAddress(address, i)) {
        #ifdef debug
          printf("\nUnable to find address for Device %d", i);
        #endif
      } else {
        sprintf(
            strAddr,
            "{0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}",
            address[0],
            address[1],
            address[2],
            address[3],
            address[4],
            address[5],
            address[6],
            address[7]);
        #ifdef debug
          //printf( "Index %d - address %s", i, strAddr);
          Serial.print("Index ");
          Serial.print(i);
          Serial.print(" - address: ");

          Serial.println(PrintHex8(address,'\0',8));
        #endif
        sensors.setResolution(address, 12);
      }
      delay(0);
    }
    sensors.setWaitForConversion(true);
    sensors.requestTemperatures();
    sensors.setWaitForConversion(false);
  }

  int8_t getIndex(uint8_t *deviceAddress) {
    DeviceAddress address;
    for (int i = 0; i < sensors.getDeviceCount(); i++) {
      if (sensors.getAddress(address, i)) {
        bool found = true;
        for (int j = 0; j < 8; j++) {
          if (deviceAddress[j] != address[j]) {
            found = false;
          }
        }
        if (found) {
          return i;
        }
      }
    }
    return -1;
  }

  uint8_t pin;
  OneWireBus *nextBus;
  unsigned long lastReadTime;
  DallasTemperature sensors;

 protected:
  OneWire oneWire;
};

class DS18B20  {

  public: DS18B20(uint8_t pin, String nameset, uint8_t *deviceAddress = nullptr ) {
    //HASensor DS18B20HA(char *deviceAddress);
      #ifdef debug
        printf("\nStarting Creating OneWire bus for pin: %d.  ", pin);
      #endif
    DS18B20::nameset=nameset;
    OneWireBus *bus = oneWireBus;
    OneWireBus *prevBus = nullptr;
    address[0] = 0;
      #ifdef debug
        Serial.print("Assign name: "); //18B20 Nameset
        Serial.println(nameset);
      #endif
    lastValidValue = TEMPERATURE_NOT_AVAILABLE;
    retryCounter = 0;

    if (bus) {
      while (bus) {
        if (bus->pin == pin) {
          myBus = bus;
          break;
        }
        prevBus = bus;
        bus = bus->nextBus;
      }
    }

    // There is no OneWire bus created yet for this pin
    if (!bus) {
      #ifdef debug
        printf("Creating OneWire bus for pin: %d", pin);
      #endif
      myBus = new OneWireBus(pin);
      if (prevBus) {
        prevBus->nextBus = myBus;
      } else {
        oneWireBus = myBus;
      }
    } else {
      #ifdef debug
        printf("Unable Creating OneWire bus for pin: %d", pin);
      #endif
    }
    if (deviceAddress == nullptr) {
      #ifdef debug
        Serial.println(F("Device address not provided. Using device from index 0"));
      #endif
    } else {
      memcpy(address, deviceAddress, 8);
      memcpy( charaddress, deviceAddress, 8);
    }
  #ifdef debug
    Serial.print(F("Assign OneWire bus for pin: "));
    Serial.println(pin);
    Serial.print(F("Assign OneWire bus for address: "));
    dumpByteArray(address);
  #endif
  }
//  bool setValue(double value, uint8_t precision = 2);

  void iterateAlways() {
    //HASensor DS18B20HA(char *deviceAddress);
    if (myBus->lastReadTime + 90000 < millis()) {
      myBus->sensors.requestTemperatures();
      myBus->lastReadTime = millis();
    }
    if (myBus->lastReadTime + 60000 < millis() && (lastReadTime != myBus->lastReadTime)) {
      lastValidValue=getValue();
      lastReadTime = myBus->lastReadTime;
        #ifdef debug
          Serial.print("18b20 temp Iterate get value: ");
          Serial.print(nameset); Serial.print(" : ");
          Serial.println(lastValidValue);
        #endif
    }
    if (lastValidValue==TEMPERATURE_NOT_AVAILABLE) lastValidValue=getValue();
  }

  double getValue() {
      double value = TEMPERATURE_NOT_AVAILABLE;
      if (address[0] == 0) {
        value = myBus->sensors.getTempCByIndex(0);
      } else {
        value = myBus->sensors.getTempC(address);
      }
      lastReadTime = millis();

      if (value == DEVICE_DISCONNECTED_C || value == 85.0) {
        value = TEMPERATURE_NOT_AVAILABLE;
      }

      if (value == TEMPERATURE_NOT_AVAILABLE) {
        retryCounter++;
        if (retryCounter > 3) {
          retryCounter = 0;
        } else {
          value = lastValidValue;
        }
      } else {
        retryCounter = 0;
      }
      lastValidValue = value;
      #ifdef dubug
        Serial.print("18B20: ");
        Serial.println(nameset);
      #endif
      if (nameset == String(dwaterThermstat)) waterTherm=value;
      if (nameset == String(dcoThermstat)) coTherm=value;
      if (TEMPERATURE_NOT_AVAILABLE != lastValidValue) {
      #ifdef dubug
        Serial.print("18B20 wyslanie do mqtt wartosci: ");
        Serial.println(lastValidValue);
      #endif
      }

      return value;
  }
  double getlast() {
    return lastValidValue;
  }


  void Init() {
  }

 protected:
  static OneWireBus *oneWireBus;
  OneWireBus *myBus;
  DeviceAddress address;
  int8_t retryCounter;
  double lastValidValue;
  unsigned long lastReadTime;
  char charaddress[17];
  String nameset;
};

OneWireBus *DS18B20::oneWireBus = nullptr;

};  // namespace HAM
};  // namespace Supla

#endif
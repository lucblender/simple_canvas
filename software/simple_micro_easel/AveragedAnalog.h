#ifndef AVERAGED_ANALOG_H
#define AVERAGED_ANALOG_H

#include "Arduino.h"

//AVERAGE_LEN should be a multiple of 2 to optimize the division
#define AVERAGE_LEN 32

class AveragedAnalog {
public:
  AveragedAnalog() {
    this->initArray();
    thersholdValue = 2;
  }
  AveragedAnalog(uint32_t thersholdValue) {
    this->initArray();
    this->thersholdValue = thersholdValue;
  }
  ~AveragedAnalog() {}

  void updateValue(uint32_t newValue);
  bool hasValueUpdated();
  uint32_t getVal();
private:
  void initArray();
  uint32_t potValues[AVERAGE_LEN];
  uint8_t potValueIndex = 0;
  uint32_t potVal;
  uint32_t potValOld;

  uint32_t thersholdValue;
};


#endif  // AVERAGED_ANALOG_H
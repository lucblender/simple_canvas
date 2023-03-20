#include "AveragedAnalog.h"


void AveragedAnalog::updateValue(uint32_t newValue) {
  this->potValues[this->potValueIndex] = newValue;
  this->potValueIndex = (this->potValueIndex + 1) % AVERAGE_LEN;

  uint32_t potAverage = 0;

  for (int i = 0; i < AVERAGE_LEN; i++) {
    potAverage += potValues[i];
  }

  this->potVal = potAverage / AVERAGE_LEN;
}

bool AveragedAnalog::hasValueUpdated() {
  if ((abs((int)potVal - (int)potValOld) >= this->thersholdValue)) {
    this->potValOld = this->potVal;
    return true;
  } else {
    return false;
  }
}

uint32_t AveragedAnalog::getVal() {
  return this->potVal;
}


void AveragedAnalog::initArray() {
  for (int i = 0; i < AVERAGE_LEN; i++) {
    potValues[i] = 1023;
  }
}
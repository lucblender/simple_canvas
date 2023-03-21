#include "AveragedAnalog.h"


void AveragedAnalog::updateValue(uint32_t newValue) {
  this->potValues[this->potValueIndex] = newValue;
  this->potValueIndex = (this->potValueIndex + 1) % AVERAGE_LEN;
  if (this->useFloatAverage) {

    float fPotAverage = 0;

    for (int i = 0; i < AVERAGE_LEN; i++) {
      fPotAverage += potValues[i];
    }

    this->fPotVal = fPotAverage / (float)AVERAGE_LEN;

  } else {

    uint32_t potAverage = 0;

    for (int i = 0; i < AVERAGE_LEN; i++) {
      potAverage += potValues[i];
    }

    this->potVal = potAverage / AVERAGE_LEN;
  }
}

bool AveragedAnalog::hasValueUpdated() {
  if (this->useFloatAverage) {
    if ((fabs(fPotVal - fPotValOld) >= 0.2f)) {
      this->fPotValOld = this->fPotVal;
      return true;
    } else {
      return false;
    }
  } else {
    if ((abs((int)potVal - (int)potValOld) >= this->thersholdValue)) {
      this->potValOld = this->potVal;
      return true;
    } else {
      return false;
    }
  }
}

uint32_t AveragedAnalog::getVal() {
  return this->potVal;
}
float AveragedAnalog::getFVal() {
  return this->fPotVal;
}

void AveragedAnalog::initArray() {

  for (int i = 0; i < AVERAGE_LEN; i++) {
    potValues[i] = 1023;
  }
}
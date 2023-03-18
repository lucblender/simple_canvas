#include "MultiShapeAdsr.h"

void MultiShapeAdsr::Init(float sampleRate) {
  this->sampleRate = sampleRate;
  this->sampleTime = 1 / sampleRate;
}
float MultiShapeAdsr::Process(bool gate) {}
void MultiShapeAdsr::setAttackShape(uint8_t shape) {}
void MultiShapeAdsr::setReleaseShape(uint8_t shape) {}
void MultiShapeAdsr::retrigger() {}

void MultiShapeAdsr::setAttackTime(float timeInS) {
  this->attackTime = timeInS;
  this->attackSampleCount = this->attackRange * this->sampleRate;
}
void MultiShapeAdsr::setDecayTime(float timeInS) {
  this->decayTime = timeInS;
  this->decaySampleCount = this->decayRange * this->sampleRate;
}
void MultiShapeAdsr::setReleaseTime(float timeInS) {
  this->releaseTime = timeInS;
  this->releaseSampleCount = this->releaseRange * this->sampleRate;
}

float MultiShapeAdsr::interpolateRisingSlope(uint8_t slopeShape, uint32_t indexTimer, uint32_t nSample, float range, float startRise) {
  switch (slopeShape) {
    case LINEAR_SHAPE:
      return (indexTimer * (1 / nSample)) * range + startRise;
    case QUADRATIC_SHAPE:
      return ((squareFloat(indexTimer / nSample)) * range + startRise);
    case QUADRATIC_INVERT_SHAPE:
      return ((1 - squareFloat((nSample - indexTimer) / nSample))) * range + startRise;
    case LOGISTIC_SHAPE:
      return (1 - 1 / (1 + expf(-5 + (indexTimer / (nSample / 10))))) * range + startRise;
    default:
      return 0.0f;
  }
}

float MultiShapeAdsr::interpolateFallingSlope(uint8_t slopeShape, uint32_t indexTimer, uint32_t nSample, float range, float endFall) {
  switch (slopeShape) {
    case LINEAR_SHAPE:
      return (1 - indexTimer * (1 / nSample)) * range + endFall;
    case QUADRATIC_SHAPE:
      return ((squareFloat((nSample - indexTimer) / nSample)) * range + endFall);
    case QUADRATIC_INVERT_SHAPE:
      return ((1 - squareFloat(indexTimer / nSample)) * range + endFall);
    case LOGISTIC_SHAPE:
      return (1 / (1 + expf(-5 + (indexTimer / (nSample / 10))))) * range + endFall;
    default:
      return 0.0f;
  }
}


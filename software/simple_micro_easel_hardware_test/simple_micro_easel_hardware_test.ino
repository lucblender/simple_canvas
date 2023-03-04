#include "DaisyDuino.h"
using namespace daisysp;

#include <Wire.h>
#include "Adafruit_MPR121.h"

#include <Adafruit_NeoPixel.h>


#define AN_SEQUENCER_STEPANALOGIN A0
#define AN_CLOCK_RATE A1
#define AN_PULSER_PERIOD A2
#define AN_MODOSC_FREQUENCY A3
#define AN_MODOSC_WAVEFORM A4
#define AN_MODOSC_ATTENUATOR A5
#define AN_COMPLEXOSC_TIMBRE A6
#define AN_COMPLEXOSC_FREQUENCY A7
#define AN_COMPLEXOSC_ATTENUATOR A8
#define AN_ENVELOPEGEN_SIG0DECAY A9
#define AN_ENVELOPEGEN_SIG1DECAY A10
#define AN_ENVELOPEGEN_SLOPESHAPE A11

#define DI_MIDIIN 1
#define DI_MIDIOUT 2
#define DI_SEQUENCER_STEPSELECT0 4
#define DI_SEQUENCER_STEPSELECT1 3
#define DI_SEQUENCER_STEPSELECT2 29
#define DI_SEQUENCER_STEPSELECT3 30
#define DI_SEQUENCER_STEPSELECT4 0
#define DI_SEQUENCER_STEP4 5
#define DI_SEQUENCER_STEP3 6
#define DI_SEQUENCER_STEP2 7
#define DI_SEQUENCER_STEP1 8
#define DI_SEQUENCER_STEP0 9
#define DI_PATCH_IRQ 10
#define DI_SCL 11
#define DI_SDA 12
#define DI_INTB 13
#define DI_INTA 14
#define DI_LEDS_DIN 26

uint8_t DI_SEQUENCER_STEPSELECT[5] = { DI_SEQUENCER_STEPSELECT0, DI_SEQUENCER_STEPSELECT1, DI_SEQUENCER_STEPSELECT2, DI_SEQUENCER_STEPSELECT3, DI_SEQUENCER_STEPSELECT4 };

float sequencerStepAnalogIn[5];
float clockRate;
float envelopeGenSig1Decay;
float envelopeGenSlopeShape;
float pulserPeriod;
float modOscFrequency;
float modOscWaveform;
float modOscAttenuator;
float complexOscTimbre;
float complexOscFrequency;
float complexOscAttenuator;
float envelopeGenSig0Decay;



// ----------------- Capacitive sensor ---------------------------
Adafruit_MPR121 cap = Adafruit_MPR121();

// ----------------- Neopixels -----------------------------------

#define NUMPIXELS 13

Adafruit_NeoPixel pixels(NUMPIXELS, DI_LEDS_DIN, NEO_RGB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  for (int i = 0; i < 5; i++) {

    pinMode(DI_SEQUENCER_STEPSELECT[i], OUTPUT);
    digitalWrite(DI_SEQUENCER_STEPSELECT[i], 0);
  }

  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

  pixels.begin();
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.fill(pixels.Color(255,0,0));
  pixels.show();

  // DAISY SETUP
  DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  DAISY.begin(ProcessAudio);
}

void loop() {
  // put your main code here, to run repeatedly:

  //sequencerStepsRead();


  clockRate = simpleAnalogRead(AN_CLOCK_RATE);
  envelopeGenSig1Decay = simpleAnalogRead(AN_PULSER_PERIOD);
  envelopeGenSlopeShape = simpleAnalogRead(AN_MODOSC_FREQUENCY);
  pulserPeriod = simpleAnalogRead(AN_MODOSC_WAVEFORM);
  modOscFrequency = simpleAnalogRead(AN_MODOSC_ATTENUATOR);
  modOscWaveform = simpleAnalogRead(AN_COMPLEXOSC_TIMBRE);
  modOscAttenuator = simpleAnalogRead(AN_COMPLEXOSC_FREQUENCY);
  complexOscTimbre = simpleAnalogRead(AN_COMPLEXOSC_ATTENUATOR);
  complexOscFrequency = simpleAnalogRead(AN_ENVELOPEGEN_SIG0DECAY);
  complexOscAttenuator = simpleAnalogRead(AN_ENVELOPEGEN_SIG1DECAY);
  envelopeGenSig0Decay = simpleAnalogRead(AN_ENVELOPEGEN_SLOPESHAPE);

  uint16_t mpr121Touched = cap.touched();
  Serial.println(mpr121Touched, BIN);

  Serial.print("clockRate ");
  Serial.print(clockRate);
  Serial.print("envelopeGenSig1Decay ");
  Serial.print(envelopeGenSig1Decay);
  Serial.print("envelopeGenSlopeShape ");
  Serial.print(envelopeGenSlopeShape);
  Serial.print("pulserPeriod ");
  Serial.print(pulserPeriod);
  Serial.print("modOscFrequency ");
  Serial.print(modOscFrequency);
  Serial.print("modOscWaveform ");
  Serial.print(modOscWaveform);
  Serial.print("modOscAttenuator ");
  Serial.print(modOscAttenuator);
  Serial.print("complexOscTimbre ");
  Serial.print(complexOscTimbre);
  Serial.print("complexOscFrequency ");
  Serial.print(complexOscFrequency);
  Serial.print("complexOscAttenuator ");
  Serial.print(complexOscAttenuator);
  Serial.print("envelopeGenSig0Decay ");
  Serial.println(envelopeGenSig0Decay);
  
  delay(100);
}

void ProcessAudio(float **in, float **out, size_t size) {
}

void sequencerStepsRead() {

  //start with all pins to 0
  for (int i = 0; i < 5; i++) {
    
    pinMode(DI_SEQUENCER_STEPSELECT[i], INPUT);
  }


  for (int indexRead = 0; indexRead < 5; indexRead++) {
    for (int i = 0; i < 5; i++) {
      pinMode(DI_SEQUENCER_STEPSELECT[i], INPUT);
    }    
    
    delay(10);

    pinMode(DI_SEQUENCER_STEPSELECT[indexRead], OUTPUT);
    digitalWrite(DI_SEQUENCER_STEPSELECT[indexRead], 1);
    sequencerStepAnalogIn[indexRead] = analogRead(AN_SEQUENCER_STEPANALOGIN);
    Serial.print("Read step ");
    Serial.print(indexRead);
    Serial.print(" : ");
    Serial.print(sequencerStepAnalogIn[indexRead]);
    Serial.print(" ");
  }
  Serial.println();
}

float semitone_to_hertz(int8_t note_number) {
  return 220 * pow(2, ((float)note_number - 0) / 12);
}

float simpleAnalogRead(uint32_t pin) {
  return (1023.0 - (float)analogRead(pin)) / 1023.0;
}

// Reads a simple pot and maps it to a value bewtween to integer values
float simpleAnalogReadAndMap(uint32_t pin, long min, long max) {
  return map(1023 - analogRead(pin), 0, 1023, min, max);
}

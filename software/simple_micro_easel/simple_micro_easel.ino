#include "DaisyDuino.h"
using namespace daisysp;

#include <Wire.h>
#include "Adafruit_MPR121.h"

#include <Adafruit_NeoPixel.h>

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTDEC(x) Serial.print(x, DEC)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#endif


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

// ----------------- Seed modules --------------------------------
static Oscillator osc;

float frequency = 440;
float sample_rate;

// ----------------- Neopixels -----------------------------------

#define NUMPIXELS 13

Adafruit_NeoPixel pixels(NUMPIXELS, DI_LEDS_DIN, NEO_GRB + NEO_KHZ800);

// ----------------- Capacitive sensor ---------------------------
#define THRESHOLD_TOUCHED 200
#define INPUT_TOUCH_COUNT 4
#define OUTPUT_TOUCH_COUNT 5
#define TOTAL_TOUCH_COUNT (INPUT_TOUCH_COUNT + OUTPUT_TOUCH_COUNT)

enum SOURCE_MODULE { NONE_SOURCE = -1,
                     SEQUENCER = 0,
                     PULSER = 1,
                     RANDOM = 2,
                     ENVELOPE_B = 3 };

enum DESTINATION_MODULE { NONE_DEST = -1,
                          OSC_A_FRQ = 0,
                          OSC_A_TMBR = 1,
                          OSC_B_FRQ = 2,
                          OSC_B_FORM = 3,
                          OSC_B_ATT = 4 };

uint16_t capacitiveSensorRawValue[TOTAL_TOUCH_COUNT];
uint8_t capacitiveSensorTouched[TOTAL_TOUCH_COUNT];
uint8_t capacitiveSensorTouchedOld[TOTAL_TOUCH_COUNT];

int8_t destinationPatches[OUTPUT_TOUCH_COUNT] = {
  NONE_DEST,
  NONE_DEST,
  NONE_DEST,
  NONE_DEST,
  NONE_DEST
};

int8_t highlightedSource = NONE_SOURCE;

Adafruit_MPR121 cap = Adafruit_MPR121();

enum {
  IDLE,
  WAIT,
  INPUT_PRESSED,
  WAIT_OUTPUT,
  OUTPUT_PRESSED,
  INPUT_CANCELLED

} capacitiveState = IDLE;

int lastCapacitiveState = IDLE;

int8_t inputPressedIndex;
int8_t outputPressedIndex;

//Blue
uint32_t sequencerColor = pixels.Color(0, 0, 120);
uint32_t sequencerColorHighlighted = pixels.Color(0, 0, 255);
//Yellow
uint32_t pulserColor = pixels.Color(100, 100, 0);
uint32_t pulserColorHighlighted = pixels.Color(200, 200, 0);
//White
uint32_t randomColor = pixels.Color(100, 100, 100);
uint32_t randomColorHighlighted = pixels.Color(200, 200, 200);
//Green
uint32_t envelopesColor = pixels.Color(0, 120, 0);
uint32_t envelopesColorHighlighted = pixels.Color(0, 255, 0);

uint32_t noneColor = pixels.Color(0, 0, 0);

uint32_t sourceColor[5] = { sequencerColor, pulserColor, randomColor, envelopesColor };
uint32_t sourceColorHighlighted[5] = { sequencerColorHighlighted, pulserColorHighlighted, randomColorHighlighted, envelopesColorHighlighted };

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);




  pinMode(DI_PATCH_IRQ, INPUT);

  for (int i = 0; i < 5; i++) {

    pinMode(DI_SEQUENCER_STEPSELECT[i], OUTPUT);
    digitalWrite(DI_SEQUENCER_STEPSELECT[i], 0);
  }

  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("MPR121 found!");

  pixels.begin();
  pixels.clear();  // Set all pixel colors to 'off'

  for (int i = 0; i < OUTPUT_TOUCH_COUNT; i++) {
    pixels.setPixelColor(i, sourceColor[i]);
  }

  pixels.show();

  // DAISY SETUP
  DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  sample_rate = DAISY.get_samplerate();

  osc.Init(sample_rate);
  osc.SetWaveform(osc.WAVE_SIN);
  osc.SetAmp(1);
  osc.SetFreq(440);

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


  capacitiveStateMachine();



  delay(10);
}

void ProcessAudio(float **in, float **out, size_t size) {
  for (size_t i = 0; i < size; i++) {
    float sample = osc.Process();
    out[0][i] = sample;
    out[1][i] = sample;
  }
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

int8_t capacitiveSensorTouch() {

  int8_t touchedIndex = -1;

  // get sensor value
  for (int i = 0; i < TOTAL_TOUCH_COUNT; i++) {
    capacitiveSensorRawValue[i] = cap.filteredData(i);
    if (capacitiveSensorRawValue[i] > THRESHOLD_TOUCHED) {
      capacitiveSensorTouched[i] = 0;
    } else {
      capacitiveSensorTouched[i] = 1;
    }
  }

  //detect if we have a touch
  for (int i = 0; i < TOTAL_TOUCH_COUNT; i++) {
    if (capacitiveSensorTouchedOld[i] != capacitiveSensorTouched[i] && capacitiveSensorTouched[i] == 1) {
      DEBUG_PRINT("Sensor n°");
      DEBUG_PRINT(i);
      DEBUG_PRINTLN(" touched");
      touchedIndex = i;
    }
  }

  //stor sensor value
  for (int i = 0; i < TOTAL_TOUCH_COUNT; i++) {
    capacitiveSensorTouchedOld[i] = capacitiveSensorTouched[i];
  }
  return touchedIndex;
}

void capacitiveStateMachine() {

  /*
enum SOURCE_MODULE { NONE = -1,
                     SEQUENCER = 0,
                     PULSER = 1,
                     RANDOM = 2,
                     ENVELOPE_B = 3 };
enum DESTINATION_MODULE { NONE = -1,
                          OSC_A_FRQ = 0,
                          OSC_A_TMBR = 1,
                          OSC_B_FRQ = 2,
                          OSC_B_FORM = 3,
                          OSC_B_ATT = 4 };
int8_t destinationPatches[OUTPUT_TOUCH_COUNT] = { DESTINATION_MODULE.NONE, DESTINATION_MODULE.NONE, DESTINATION_MODULE.NONE, DESTINATION_MODULE.NONE, DESTINATION_MODULE.NONE };
int8_t highlightedSource = SOURCE_MODULE.NONE;


uint32_t sourceColor[5] = {sequencerColor, pulserColor, randomColor, envelopesColor};
uint32_t sourceColorHighlighted[5] = {sequencerColorHighlighted, pulserColorHighlighted, randomColorHighlighted, envelopesColorHighlighted};
*/

  //TRANSITION
  switch (lastCapacitiveState) {
    case IDLE:
      capacitiveState = WAIT;
      break;

    case WAIT:
      {
        int8_t sensorTouchIndex = capacitiveSensorTouch();
        if (sensorTouchIndex > -1 && sensorTouchIndex < INPUT_TOUCH_COUNT) {
          capacitiveState = INPUT_PRESSED;
          inputPressedIndex = sensorTouchIndex;  //goes from 0 to 4
        }
        break;
      }

    case INPUT_PRESSED:
      capacitiveState = WAIT_OUTPUT;
      break;

    case WAIT_OUTPUT:
      {
        int8_t sensorTouchIndex = capacitiveSensorTouch();
        if (sensorTouchIndex >= INPUT_TOUCH_COUNT && sensorTouchIndex < (INPUT_TOUCH_COUNT + OUTPUT_TOUCH_COUNT)) {
          capacitiveState = OUTPUT_PRESSED;
          outputPressedIndex = sensorTouchIndex - INPUT_TOUCH_COUNT;  //so it goes from 0 to 5

        } else if (sensorTouchIndex == inputPressedIndex) {  //can only cancel if we repress the same input
          capacitiveState = INPUT_CANCELLED;
        }
        break;
      }

    case OUTPUT_PRESSED:
      capacitiveState = WAIT;
      break;

    case INPUT_CANCELLED:
      capacitiveState = WAIT;
      break;

    default:
      //Error
      break;
  };

  //CODE
  if (lastCapacitiveState != capacitiveState) {
    switch (capacitiveState) {
      case IDLE:
        DEBUG_PRINTLN("Enter IDLE State");
        break;
      case WAIT:
        DEBUG_PRINTLN("Enter WAIT State");
        break;
      case INPUT_PRESSED:
        DEBUG_PRINTLN("Enter INPUT_PRESSED State");

        pixels.setPixelColor(inputPressedIndex, sourceColorHighlighted[inputPressedIndex]);
        highlightedSource = SOURCE_MODULE(inputPressedIndex);
        pixels.show();

        break;
      case WAIT_OUTPUT:
        DEBUG_PRINTLN("Enter WAIT_OUTPUT State");
        break;
      case OUTPUT_PRESSED:
        DEBUG_PRINTLN("Enter OUTPUT_PRESSED State");
        if (destinationPatches[outputPressedIndex] == SOURCE_MODULE(inputPressedIndex)) {
          // repressed an existing pass --> cancel the patch
          destinationPatches[outputPressedIndex] = NONE_SOURCE;
          pixels.setPixelColor(outputPressedIndex + INPUT_TOUCH_COUNT, noneColor);
          pixels.show();
        } else {
          destinationPatches[outputPressedIndex] = SOURCE_MODULE(inputPressedIndex);

          pixels.setPixelColor(outputPressedIndex + INPUT_TOUCH_COUNT, sourceColor[inputPressedIndex]);
          pixels.show();
        }


        pixels.setPixelColor(inputPressedIndex, sourceColor[inputPressedIndex]);
        pixels.show();
        highlightedSource = NONE_SOURCE;

        break;
      case INPUT_CANCELLED:
        DEBUG_PRINTLN("Enter INPUT_CANCELLED State");

        pixels.setPixelColor(inputPressedIndex, sourceColor[inputPressedIndex]);
        pixels.show();
        highlightedSource = NONE_SOURCE;
        break;

      default:
        DEBUG_PRINTLN("Enter default State");
        //Error
        break;
    };
  }
  lastCapacitiveState = capacitiveState;
}

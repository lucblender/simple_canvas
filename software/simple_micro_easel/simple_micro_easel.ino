#include "DaisyDuino.h"
using namespace daisysp;

#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <Adafruit_MCP23X17.h>

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


#define TRIGGER_DIFF 0.01
#define DEFAULT_VALUE 5  //put a default value that is not possible to be sure all pins are initialized correctly

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

//Digital input gpio definition
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

//Digital input gpio expander gpio definition
#define DI_SEQUENCER_TRIGGER 0
#define DI_SEQUENCER_STAGE0 1
#define DI_SEQUENCER_STAGE1 2
#define DI_RANDOM_TRIGGERSELECT0 3
#define DI_RANDOM_TRIGGERSELECT1 4
#define DI_PULSER_TRIGGERSELECT0 5
#define DI_PULSER_TRIGGERSELECT1 6
#define DI_MODOSC_AMFM 7
#define DI_COMPLEXOSC_WAVEFORM0 8
#define DI_COMPLEXOSC_WAVEFORM1 9
#define DI_ENVELOPEGEN_SIG0SELECTOR0 10
#define DI_ENVELOPEGEN_SIG0SELECTOR1 11
#define DI_ENVELOPEGEN_SIG1SELECTOR0 12
#define DI_ENVELOPEGEN_SIG1SELECTOR1 13
#define DI_ENVELOPEGEN_SIG0LPGVCA 14
#define DI_ENVELOPEGEN_SIG1LPGVCA 15

uint8_t DI_SEQUENCER_STEPSELECT[5] = { DI_SEQUENCER_STEPSELECT0, DI_SEQUENCER_STEPSELECT1, DI_SEQUENCER_STEPSELECT2, DI_SEQUENCER_STEPSELECT3, DI_SEQUENCER_STEPSELECT4 };

//analog pins value
float sequencerStepAnalogIn[5];
float clockRate;
float envelopeGenSig1Decay;
int envelopeGenSlopeShape;
float pulserPeriod;
float modOscFrequency;
float modOscWaveform;
float modOscAttenuator;
float complexOscTimbre;
float complexOscFrequency;
float complexOscAttenuator;
float envelopeGenSig0Decay;

float modOscFrequencyOld = DEFAULT_VALUE;
float complexOscFrequencyOld = DEFAULT_VALUE;
float modOscWaveformOld = DEFAULT_VALUE;

// ditial pins value
uint8_t sequencerStep4 = 0;
uint8_t sequencerStep3 = 0;
uint8_t sequencerStep2 = 0;
uint8_t sequencerStep1 = 0;
uint8_t sequencerStep0 = 0;

uint8_t sequencerStep4Old = DEFAULT_VALUE;
uint8_t sequencerStep3Old = DEFAULT_VALUE;
uint8_t sequencerStep2Old = DEFAULT_VALUE;
uint8_t sequencerStep1Old = DEFAULT_VALUE;
uint8_t sequencerStep0Old = DEFAULT_VALUE;

// mcp pins value
uint8_t sequencerTrigger = 0;
uint8_t sequencerStage = 0;
uint8_t randomTriggerSelect = 0;
uint8_t pulserTriggerSelect = 0;
uint8_t modOscAmFm = 0;
uint8_t complexOscWaveform = 0;
uint8_t envelopeGenSig0Selector = 0;
uint8_t envelopeGenSig0LpgVca = 0;
uint8_t envelopeGenSig1Selector = 0;
uint8_t envelopeGenSig1LpgVca = 0;

uint8_t sequencerTriggerOld = DEFAULT_VALUE;
uint8_t sequencerStageOld = DEFAULT_VALUE;
uint8_t randomTriggerSelectOld = DEFAULT_VALUE;
uint8_t pulserTriggerSelectOld = DEFAULT_VALUE;
uint8_t modOscAmFmOld = DEFAULT_VALUE;
uint8_t complexOscWaveformOld = DEFAULT_VALUE;
uint8_t envelopeGenSig0SelectorOld = DEFAULT_VALUE;
uint8_t envelopeGenSig0LpgVcaOld = DEFAULT_VALUE;
uint8_t envelopeGenSig1SelectorOld = DEFAULT_VALUE;
uint8_t envelopeGenSig1LpgVcaOld = DEFAULT_VALUE;

// --------------------- Gpio expander --------------------------
Adafruit_MCP23X17 mcp;

// ----------------- Seed modules --------------------------------

static Oscillator complexOscSinus;
static VariableShapeOscillator complexOscBasis;

static VariableShapeOscillator modulationOsc;

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

  if (!mcp.begin_I2C()) {
    Serial.println("MCP not found, check wiring?");
    while (1)
      ;
  }
  Serial.println("MCP found!");

  mcp.pinMode(DI_SEQUENCER_TRIGGER, INPUT_PULLUP);
  mcp.pinMode(DI_SEQUENCER_STAGE0, INPUT_PULLUP);
  mcp.pinMode(DI_SEQUENCER_STAGE1, INPUT_PULLUP);
  mcp.pinMode(DI_RANDOM_TRIGGERSELECT0, INPUT_PULLUP);
  mcp.pinMode(DI_RANDOM_TRIGGERSELECT1, INPUT_PULLUP);
  mcp.pinMode(DI_PULSER_TRIGGERSELECT0, INPUT_PULLUP);
  mcp.pinMode(DI_PULSER_TRIGGERSELECT1, INPUT_PULLUP);
  mcp.pinMode(DI_MODOSC_AMFM, INPUT_PULLUP);
  mcp.pinMode(DI_COMPLEXOSC_WAVEFORM0, INPUT_PULLUP);
  mcp.pinMode(DI_COMPLEXOSC_WAVEFORM1, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG0SELECTOR0, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG0SELECTOR1, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG0LPGVCA, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG1SELECTOR0, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG1SELECTOR1, INPUT_PULLUP);
  mcp.pinMode(DI_ENVELOPEGEN_SIG1LPGVCA, INPUT_PULLUP);

  pinMode(DI_INTB, INPUT);
  pinMode(DI_INTA, INPUT);

  pinMode(DI_SEQUENCER_STEP4, INPUT_PULLUP);
  pinMode(DI_SEQUENCER_STEP3, INPUT_PULLUP);
  pinMode(DI_SEQUENCER_STEP2, INPUT_PULLUP);
  pinMode(DI_SEQUENCER_STEP1, INPUT_PULLUP);
  pinMode(DI_SEQUENCER_STEP0, INPUT_PULLUP);
  pixels.begin();
  pixels.clear();  // Set all pixel colors to 'off'

  for (int i = 0; i < OUTPUT_TOUCH_COUNT; i++) {
    pixels.setPixelColor(i, sourceColor[i]);
  }

  pixels.show();

  // DAISY SETUP
  DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  sample_rate = DAISY.get_samplerate();

  // init complex oscillators
  complexOscSinus.Init(sample_rate);
  complexOscSinus.SetWaveform(complexOscSinus.WAVE_SIN);
  complexOscSinus.SetAmp(1);

  complexOscBasis.Init(sample_rate);
  setComplexOscillatorFrequency(220);

  modulationOsc.Init(sample_rate);
  setModulationOscillatorFrequency(220);

//read onces all gpios before starting daisy
  digitalPinsread();
  mcpPinsRead();
  analogsRead();

  DAISY.begin(ProcessAudio);
}

void loop() {
  capacitiveStateMachine();
  digitalPinsread();
  mcpPinsRead();
  analogsRead();

  delay(10);
}

void ProcessAudio(float **in, float **out, size_t size) {
  for (size_t i = 0; i < size; i++) {

    float modulationOscSample = modulationOsc.Process();
    
    float attenuatedModulationOscSample = modulationOscSample * modOscAttenuator;
    float attenuatedComplexMixed;

    if (modOscAmFm == 0) {  //0 = AM

      // si attenuator = 0 --> signal sans modulation  signal = attenuatedComplexMixed
      // si attenuator = 1 --> signal modulation max  signal = attenuatedComplexMixed * 100% of (attenuatedModulationOscSample)

      //si attenuator = 0 --> signal

      // modulationOscSample oscillate -1..1
      // modOscAttenuator 0..1 gain 

      float gain = 1.0f-(((modulationOscSample+1.0f)/2)*modOscAttenuator);


      float complexSinusSample = complexOscSinus.Process();
      float complexBasisSample = complexOscBasis.Process();

      float complexMixed = (1.0 - complexOscTimbre) * complexSinusSample + complexOscTimbre * complexBasisSample;
      attenuatedComplexMixed = complexMixed * complexOscAttenuator*gain;
    } else {  // 1 = FM

      // modulationOscSample oscillate -1..1
      //(modulationOscSample * 0.5f) -0.5 .. 0.5
      // modOscAttenuator 0..1 gain 
      // fmFrequencyRamp --> example complexOscFrequency = 400hz --> 400 * 0.1 * ((-0.5..0.5)*0..1)

      float fmFrequencyRamp = complexOscFrequency * 0.1 * ((modulationOscSample * 0.5f)*modOscAttenuator);
      setComplexOscillatorFrequency(complexOscFrequency + fmFrequencyRamp);
      float complexSinusSample = complexOscSinus.Process();
      float complexBasisSample = complexOscBasis.Process();

      float complexMixed = (1.0 - complexOscTimbre) * complexSinusSample + complexOscTimbre * complexBasisSample;
      attenuatedComplexMixed = complexMixed * complexOscAttenuator;
    }



    out[0][i] = attenuatedComplexMixed;
    out[1][i] = attenuatedComplexMixed;
  }
}

void digitalPinsread() {
  sequencerStep0 = digitalRead(DI_SEQUENCER_STEP0);
  sequencerStep1 = digitalRead(DI_SEQUENCER_STEP1);
  sequencerStep2 = digitalRead(DI_SEQUENCER_STEP2);
  sequencerStep3 = digitalRead(DI_SEQUENCER_STEP3);
  sequencerStep4 = digitalRead(DI_SEQUENCER_STEP4);

  if (sequencerStep0Old != sequencerStep0) {  //0 disabled 1 enabled
    Serial.print("sequencerStep0 ");
    Serial.println(sequencerStep0);
  };
  if (sequencerStep1Old != sequencerStep1) {  //0 disabled 1 enabled
    Serial.print("sequencerStep1 ");
    Serial.println(sequencerStep1);
  };
  if (sequencerStep2Old != sequencerStep2) {  //0 disabled 1 enabled
    Serial.print("sequencerStep2 ");
    Serial.println(sequencerStep2);
  };
  if (sequencerStep3Old != sequencerStep3) {  //0 disabled 1 enabled
    Serial.print("sequencerStep3 ");
    Serial.println(sequencerStep3);
  };
  if (sequencerStep4Old != sequencerStep4) {  //0 disabled 1 enabled
    Serial.print("sequencerStep4 ");
    Serial.println(sequencerStep4);
  };

  sequencerStep0Old = sequencerStep0;
  sequencerStep1Old = sequencerStep1;
  sequencerStep2Old = sequencerStep2;
  sequencerStep3Old = sequencerStep3;
  sequencerStep4Old = sequencerStep4;
}

void mcpPinsRead() {
  sequencerTrigger = mcp.digitalRead(DI_SEQUENCER_TRIGGER);
  sequencerStage = mcp.digitalRead(DI_SEQUENCER_STAGE0) + 2 * mcp.digitalRead(DI_SEQUENCER_STAGE1);
  randomTriggerSelect = mcp.digitalRead(DI_RANDOM_TRIGGERSELECT0) + 2 * mcp.digitalRead(DI_RANDOM_TRIGGERSELECT1);
  pulserTriggerSelect = mcp.digitalRead(DI_PULSER_TRIGGERSELECT0) + 2 * mcp.digitalRead(DI_PULSER_TRIGGERSELECT1);
  modOscAmFm = mcp.digitalRead(DI_MODOSC_AMFM);
  complexOscWaveform = mcp.digitalRead(DI_COMPLEXOSC_WAVEFORM0) + 2 * mcp.digitalRead(DI_COMPLEXOSC_WAVEFORM1);
  envelopeGenSig0Selector = mcp.digitalRead(DI_ENVELOPEGEN_SIG0SELECTOR0) + 2 * mcp.digitalRead(DI_ENVELOPEGEN_SIG0SELECTOR1);
  envelopeGenSig0LpgVca = mcp.digitalRead(DI_ENVELOPEGEN_SIG0LPGVCA);
  envelopeGenSig1Selector = mcp.digitalRead(DI_ENVELOPEGEN_SIG1SELECTOR0) + 2 * mcp.digitalRead(DI_ENVELOPEGEN_SIG1SELECTOR1);
  envelopeGenSig1LpgVca = mcp.digitalRead(DI_ENVELOPEGEN_SIG1LPGVCA);

  if (sequencerTriggerOld != sequencerTrigger) {  // 0 clock, 1 pulser
    Serial.print("sequencerTrigger ");
    Serial.println(sequencerTrigger);
  }
  if (sequencerStageOld != sequencerStage) {  // 2 = 3steps, 3 = 4steps, 1 = 5steps
    Serial.print("sequencerStage ");
    Serial.println(sequencerStage);
  }
  if (randomTriggerSelectOld != randomTriggerSelect) {  //2 = pulser, 3 = clock, 1 = sequencer
    Serial.print("randomTriggerSelect ");
    Serial.println(randomTriggerSelect);
  }
  if (pulserTriggerSelectOld != pulserTriggerSelect) {  //2 = pulser, 3 = clock, 1 = sequencer
    Serial.print("pulserTriggerSelect ");
    Serial.println(pulserTriggerSelect);
  }
  if (modOscAmFmOld != modOscAmFm) {  //0 = AM, 1 = FM
    Serial.print("modOscAmFm ");
    Serial.println(modOscAmFm);
  }

  if (complexOscWaveformOld != complexOscWaveform) {  // 2 = small pulse, 3 = square, 1 = triangle
    Serial.print("complexOscWaveform ");
    Serial.println(complexOscWaveform);

    if (complexOscWaveform == 2) {
      complexOscBasis.SetPW(0.95f);
      complexOscBasis.SetWaveshape(1.0f);  // 1 = square
    } else if (complexOscWaveform == 3) {
      complexOscBasis.SetPW(0.5f);
      complexOscBasis.SetWaveshape(1.0f);
    } else if (complexOscWaveform == 1) {
      complexOscBasis.SetPW(0.5f);
      complexOscBasis.SetWaveshape(0.0f);  //0 = triangle
    }
  }

  if (envelopeGenSig0SelectorOld != envelopeGenSig0Selector) {  // 2 = pulser, 3 = clock, 1 = sequencer
    Serial.print("envelopeGenSig0Selector ");
    Serial.println(envelopeGenSig0Selector);
  }
  if (envelopeGenSig0LpgVcaOld != envelopeGenSig0LpgVca) {  // 0 = LPG, 1 = VCA
    Serial.print("envelopeGenSig0LpgVca ");
    Serial.println(envelopeGenSig0LpgVca);
  }
  if (envelopeGenSig1SelectorOld != envelopeGenSig1Selector) {  // 2 = pulser, 3 = clock, 1 = sequencer
    Serial.print("envelopeGenSig1Selector ");
    Serial.println(envelopeGenSig1Selector);
  }
  if (envelopeGenSig1LpgVcaOld != envelopeGenSig1LpgVca) {  // 0 = LPG, 1 = VCA
    Serial.print("envelopeGenSig1LpgVca ");
    Serial.println(envelopeGenSig1LpgVca);
  }

  sequencerTriggerOld = sequencerTrigger;
  sequencerStageOld = sequencerStage;
  randomTriggerSelectOld = randomTriggerSelect;
  pulserTriggerSelectOld = pulserTriggerSelect;
  modOscAmFmOld = modOscAmFm;
  complexOscWaveformOld = complexOscWaveform;
  envelopeGenSig0SelectorOld = envelopeGenSig0Selector;
  envelopeGenSig0LpgVcaOld = envelopeGenSig0LpgVca;
  envelopeGenSig1SelectorOld = envelopeGenSig1Selector;
  envelopeGenSig1LpgVcaOld = envelopeGenSig1LpgVca;
}

void analogsRead() {
  clockRate = simpleAnalogRead(AN_CLOCK_RATE);
  pulserPeriod = simpleAnalogRead(AN_PULSER_PERIOD);
  modOscFrequency = modOscFrequency * 0.9 + 0.1 * simpleAnalogReadAndMap(AN_MODOSC_FREQUENCY, 0, 8000);
  modOscWaveform = simpleAnalogRead(AN_MODOSC_WAVEFORM);
  modOscAttenuator = simpleAnalogRead(AN_MODOSC_ATTENUATOR);
  complexOscTimbre = simpleAnalogRead(AN_COMPLEXOSC_TIMBRE);
  complexOscFrequency = complexOscFrequency * 0.9 + 0.1 * simpleAnalogReadAndMap(AN_COMPLEXOSC_FREQUENCY, 0, 8000);
  complexOscAttenuator = simpleAnalogRead(AN_COMPLEXOSC_ATTENUATOR);
  envelopeGenSig0Decay = simpleAnalogRead(AN_ENVELOPEGEN_SIG0DECAY);
  envelopeGenSig1Decay = simpleAnalogRead(AN_ENVELOPEGEN_SIG1DECAY);
  envelopeGenSlopeShape = (int)simpleAnalogReadAndMap(AN_ENVELOPEGEN_SLOPESHAPE, 0, 6.5);

  if (abs(complexOscFrequencyOld - complexOscFrequency) > 1) {  //TODO make this better
    setComplexOscillatorFrequency(complexOscFrequency);
    complexOscFrequencyOld = complexOscFrequency;
  }


  if (abs(modOscFrequencyOld - modOscFrequency) > 1) {  //TODO make this better
    setModulationOscillatorFrequency(modOscFrequencyOld);
    modOscFrequencyOld = modOscFrequency;
  }

  if (abs(modOscWaveformOld - modOscWaveform) > TRIGGER_DIFF) {
    if (modOscWaveform < 0.5) {
      //from tirangle to sawtooth 0 = triangle 0.5 = sawtooth
      // tirangle shape = 0, pw = 0.5
      //sawtooth shape =  0, pw 1
      modulationOsc.SetWaveshape(0.0f);  // 1 = square
      modulationOsc.SetPW(1.0f - (0.5f - modOscWaveform));

    } else {
      //from sawtooth to square: 0.5 = sawtooth 1 = square
      //sawtooth shape =  0, pw 1
      //square shape = 1, pw = 0.5
      modulationOsc.SetWaveshape((modOscWaveform - 0.5f) * 2);  // 1 = square
      modulationOsc.SetPW(1.0f - (modOscWaveform - 0.5f));
    }
    modOscWaveformOld = modOscWaveform;
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

void setComplexOscillatorFrequency(float frequency) {
  complexOscBasis.SetFreq(frequency);
  complexOscBasis.SetSyncFreq(frequency);
  complexOscSinus.SetFreq(frequency);
}

void setModulationOscillatorFrequency(float frequency) {
  modulationOsc.SetSyncFreq(frequency);
  modulationOsc.SetFreq(frequency);
}

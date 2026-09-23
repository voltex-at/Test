#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>

TFT_eSPI lcd;

static constexpr int PIN_BACKLIGHT = 21;
static constexpr int PIN_AUDIO_DAC = 26;
static constexpr uint32_t SAMPLE_RATE = 16000;
static constexpr uint32_t TONE_MS = 3500;
static constexpr uint32_t SILENCE_MS = 1800;

static void dacOffHiZ() {
  dacDisable(PIN_AUDIO_DAC);
  pinMode(PIN_AUDIO_DAC, INPUT);
}

static void showPhase(const char* title, const char* subtitle, uint16_t color) {
  lcd.fillRect(0, 54, 320, 160, TFT_BLACK);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString("AUDIO TEST 4", 160, 72, 4);
  lcd.setTextColor(color, TFT_BLACK);
  lcd.drawString(title, 160, 124, 4);
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  lcd.drawString(subtitle, 160, 164, 2);
}

static void playTone440(uint8_t amplitude, uint32_t durationMs) {
  dacWrite(PIN_AUDIO_DAC, 128);
  delay(80);

  const uint32_t totalSamples = (SAMPLE_RATE * durationMs) / 1000UL;
  uint32_t phase = 0;
  const uint32_t phaseStep = (uint32_t)((440.0 * 4294967296.0) / SAMPLE_RATE);
  uint32_t nextUs = micros();

  for (uint32_t i = 0; i < totalSamples; ++i) {
    phase += phaseStep;
    const float angle = (float)((phase >> 16) & 0xFFFF) * (2.0f * PI / 65536.0f);
    const int sample = 128 + (int)((float)amplitude * sinf(angle));
    dacWrite(PIN_AUDIO_DAC, constrain(sample, 0, 255));

    nextUs += 1000000UL / SAMPLE_RATE;
    while ((int32_t)(micros() - nextUs) < 0) {}
  }

  dacWrite(PIN_AUDIO_DAC, 128);
  delay(80);
}

static void runLevel(uint8_t amplitude) {
  char line1[32];
  char line2[48];

  snprintf(line1, sizeof(line1), "AMP +/- %u", amplitude);
  snprintf(line2, sizeof(line2), "440 Hz, DAC active, level %u", amplitude);

  showPhase(line1, line2, TFT_YELLOW);
  Serial.printf("TEST4: playing 440 Hz at amplitude +/- %u\n", amplitude);
  playTone440(amplitude, TONE_MS);

  dacOffHiZ();
  showPhase("SILENCE", "DAC OFF / GPIO26 Hi-Z", TFT_GREEN);
  Serial.println("TEST4: DAC OFF / GPIO26 Hi-Z");
  delay(SILENCE_MS);
}

void setup() {
  Serial.begin(115200);

  const int leds[] = {4, 16, 17};
  for (int pin : leds) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH);

  dacOffHiZ();

  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString("AUDIO TEST 4", 160, 68, 4);
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  lcd.drawString("440 Hz: +/-4 / 8 / 16 / 32", 160, 116, 2);
  lcd.drawString("Hi-Z silence between levels", 160, 146, 2);
  lcd.drawString("PWM / WiFi / SD / touch OFF", 160, 176, 2);

  delay(1500);
}

void loop() {
  digitalWrite(PIN_BACKLIGHT, HIGH);

  runLevel(4);
  runLevel(8);
  runLevel(16);
  runLevel(32);

  showPhase("CYCLE END", "Restarting levels...", TFT_CYAN);
  delay(2500);
}

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>

TFT_eSPI lcd;

static constexpr int PIN_BACKLIGHT = 21;
static constexpr int PIN_AUDIO_DAC = 26;
static constexpr uint32_t SAMPLE_RATE = 16000;
static constexpr uint32_t TONE_MS = 5000;
static constexpr uint32_t SILENCE_MS = 5000;

static void showPhase(const char* title, const char* subtitle, uint16_t color) {
  lcd.fillRect(0, 60, 320, 150, TFT_BLACK);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString("AUDIO TEST 3", 160, 78, 4);
  lcd.setTextColor(color, TFT_BLACK);
  lcd.drawString(title, 160, 128, 4);
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  lcd.drawString(subtitle, 160, 166, 2);
}

static void playTone440(uint32_t durationMs) {
  // Activate DAC at midscale first to reduce the transition pop.
  dacWrite(PIN_AUDIO_DAC, 128);
  delay(80);

  const uint32_t totalSamples = (SAMPLE_RATE * durationMs) / 1000UL;
  uint32_t phase = 0;
  const uint32_t phaseStep = (uint32_t)((440.0 * 4294967296.0) / SAMPLE_RATE);
  uint32_t nextUs = micros();

  for (uint32_t i = 0; i < totalSamples; ++i) {
    phase += phaseStep;
    const float angle = (float)((phase >> 16) & 0xFFFF) * (2.0f * PI / 65536.0f);

    // Moderate amplitude: enough to judge audio quality without being excessively loud.
    const int sample = 128 + (int)(42.0f * sinf(angle));
    dacWrite(PIN_AUDIO_DAC, constrain(sample, 0, 255));

    nextUs += 1000000UL / SAMPLE_RATE;
    while ((int32_t)(micros() - nextUs) < 0) {
      // tight timing loop for a stable diagnostic tone
    }
  }

  // Return to midscale briefly before fully disabling DAC.
  dacWrite(PIN_AUDIO_DAC, 128);
  delay(80);
}

static void dacOffHiZ() {
  dacDisable(PIN_AUDIO_DAC);
  pinMode(PIN_AUDIO_DAC, INPUT);
}

void setup() {
  Serial.begin(115200);

  const int leds[] = {4, 16, 17};
  for (int pin : leds) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  // Keep backlight static: no LEDC/PWM.
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH);

  // Start in true silence state.
  dacOffHiZ();

  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString("AUDIO TEST 3", 160, 70, 4);
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  lcd.drawString("5 s tone / 5 s DAC OFF", 160, 120, 2);
  lcd.drawString("GPIO21 static HIGH", 160, 148, 2);
  lcd.drawString("WiFi / SD / touch OFF", 160, 176, 2);
  delay(1200);
}

void loop() {
  digitalWrite(PIN_BACKLIGHT, HIGH);

  showPhase("TONE 440 Hz", "DAC26 ACTIVE - listen for buzz/noise", TFT_YELLOW);
  Serial.println("TEST3: DAC ON, playing 440 Hz for 5 seconds.");
  playTone440(TONE_MS);

  dacOffHiZ();
  showPhase("SILENCE", "DAC26 OFF / GPIO26 Hi-Z", TFT_GREEN);
  Serial.println("TEST3: DAC OFF / GPIO26 Hi-Z for 5 seconds.");
  delay(SILENCE_MS);
}

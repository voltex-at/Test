#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI lcd;

static constexpr int PIN_BACKLIGHT = 21;
static constexpr int PIN_AUDIO_DAC = 26;

void setup() {
  Serial.begin(115200);

  const int leds[] = {4, 16, 17};
  for (int pin : leds) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  // No PWM on the backlight.
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH);

  // AUDIO TEST 2: explicitly attach DAC once, then disable it,
  // and leave GPIO26 as a high-impedance input.
  dacWrite(PIN_AUDIO_DAC, 128);
  delay(20);
  dacDisable(PIN_AUDIO_DAC);
  pinMode(PIN_AUDIO_DAC, INPUT);

  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString("AUDIO TEST 2", 160, 72, 4);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.drawString("BACKLIGHT: NO PWM", 160, 114, 2);
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.drawString("GPIO26: DAC OFF / Hi-Z", 160, 144, 2);
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  lcd.drawString("WiFi / SD / touch OFF", 160, 176, 2);

  Serial.println("AUDIO TEST 2: GPIO21 HIGH, DAC26 disabled, GPIO26 INPUT Hi-Z.");
}

void loop() {
  digitalWrite(PIN_BACKLIGHT, HIGH);
  pinMode(PIN_AUDIO_DAC, INPUT);
  delay(1000);
}

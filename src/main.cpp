#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI lcd;

static constexpr int PIN_BACKLIGHT = 21;
static constexpr int PIN_AUDIO_DAC = 26;

void setup() {
  Serial.begin(115200);

  // CYD status LEDs off (active-low on common boards).
  const int leds[] = {4, 16, 17};
  for (int pin : leds) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  // Diagnostic change: NO LEDC/PWM. Backlight is a constant logic HIGH.
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH);

  // Keep the analog amplifier input at DAC midscale instead of floating.
  dacWrite(PIN_AUDIO_DAC, 128);

  lcd.init();
  lcd.setRotation(1);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextDatum(MC_DATUM);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString("AUDIO TEST", 160, 78, 4);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.drawString("BACKLIGHT: NO PWM", 160, 120, 2);
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.drawString("DAC26 = 128 (idle)", 160, 148, 2);
  lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  lcd.drawString("WiFi / SD / touch OFF", 160, 180, 2);

  Serial.println("CYD audio diagnostic: GPIO21 HIGH, no PWM; DAC26=128; WiFi/SD/touch disabled.");
}

void loop() {
  digitalWrite(PIN_BACKLIGHT, HIGH);
  dacWrite(PIN_AUDIO_DAC, 128);
  delay(1000);
}

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

  // Diagnostic: NO LEDC/PWM. Backlight is a constant logic HIGH.
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH);

  // AUDIO TEST 2:
  // Enable DAC only momentarily so dacDisable() can explicitly detach it,
  // then return GPIO26 to a high-impedance input.
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

  Serial.println("CYD AUDIO TEST 2: GPIO21 HIGH/no PWM; DAC26 disabled; GPIO26 INPUT Hi-Z; WiFi/SD/touch disabled.");
}

void loop() {
  // Keep backlight static and GPIO26 high impedance.
  digitalWrite(PIN_BACKLIGHT, HIGH);
  pinMode(PIN_AUDIO_DAC, INPUT);
  delay(1000);
}

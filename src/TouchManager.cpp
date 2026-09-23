#include "TouchManager.h"
#include "Config.h"

void TouchManager::begin() {
  pinMode(AppConfig::TOUCH_CLK, OUTPUT);
  pinMode(AppConfig::TOUCH_MOSI, OUTPUT);
  pinMode(AppConfig::TOUCH_MISO, INPUT);
  pinMode(AppConfig::TOUCH_CS, OUTPUT);
  pinMode(AppConfig::TOUCH_IRQ, INPUT);
  digitalWrite(AppConfig::TOUCH_CS, HIGH);
  digitalWrite(AppConfig::TOUCH_CLK, LOW);
}

uint8_t TouchManager::transfer8(uint8_t value) {
  uint8_t in = 0;
  for (int i = 7; i >= 0; --i) {
    digitalWrite(AppConfig::TOUCH_MOSI, (value >> i) & 0x01);
    digitalWrite(AppConfig::TOUCH_CLK, HIGH);
    delayMicroseconds(1);
    in = static_cast<uint8_t>((in << 1) | digitalRead(AppConfig::TOUCH_MISO));
    digitalWrite(AppConfig::TOUCH_CLK, LOW);
    delayMicroseconds(1);
  }
  return in;
}

uint16_t TouchManager::read12(uint8_t command) {
  digitalWrite(AppConfig::TOUCH_CS, LOW);
  transfer8(command);
  const uint16_t hi = transfer8(0x00);
  const uint16_t lo = transfer8(0x00);
  digitalWrite(AppConfig::TOUCH_CS, HIGH);
  return static_cast<uint16_t>(((hi << 8) | lo) >> 3) & 0x0FFF;
}

TouchPoint TouchManager::read() {
  TouchPoint p;
  if (digitalRead(AppConfig::TOUCH_IRQ) != LOW) return p;

  // Kilka probek i mediana/uproczone usrednienie ograniczaja drgania rezystywnego panela.
  uint32_t sx = 0, sy = 0;
  constexpr uint8_t samples = 5;
  for (uint8_t i = 0; i < samples; ++i) {
    sx += read12(0xD0); // X
    sy += read12(0x90); // Y
  }
  p.rawX = sx / samples;
  p.rawY = sy / samples;

  // Rotation=1. Te osie moga wymagac odwrocenia po fizycznym tescie konkretnej rewizji.
  long x = map(p.rawY, AppConfig::TOUCH_Y_MIN, AppConfig::TOUCH_Y_MAX, 0, 319);
  long y = map(p.rawX, AppConfig::TOUCH_X_MIN, AppConfig::TOUCH_X_MAX, 239, 0);
  p.x = constrain(x, 0L, 319L);
  p.y = constrain(y, 0L, 239L);
  p.touched = true;
  return p;
}

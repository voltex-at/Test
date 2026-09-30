#include "TouchManager.h"
#include "Config.h"
#include <Wire.h>

void TouchManager::begin() {
  pinMode(AppConfig::TOUCH_RST, OUTPUT);
  pinMode(AppConfig::TOUCH_IRQ, INPUT_PULLUP);

  digitalWrite(AppConfig::TOUCH_RST, LOW);
  delay(10);
  digitalWrite(AppConfig::TOUCH_RST, HIGH);
  delay(300);

  Wire.begin(AppConfig::I2C_SDA, AppConfig::I2C_SCL);
  Wire.setClock(400000);
}

bool TouchManager::readBytes(uint8_t reg, uint8_t* data, size_t len) {
  Wire.beginTransmission(AppConfig::TOUCH_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;

  const size_t got = Wire.requestFrom(AppConfig::TOUCH_ADDR, static_cast<uint8_t>(len));
  if (got != len) return false;
  for (size_t i = 0; i < len; ++i) data[i] = Wire.read();
  return true;
}

TouchPoint TouchManager::read() {
  TouchPoint p;

  uint8_t status = 0;
  if (!readBytes(0x02, &status, 1)) return p;
  if ((status & 0x0F) == 0) return p;

  uint8_t b[4] = {};
  if (!readBytes(0x03, b, sizeof(b))) return p;

  const uint16_t rawX = static_cast<uint16_t>(((b[0] & 0x0F) << 8) | b[1]);
  const uint16_t rawY = static_cast<uint16_t>(((b[2] & 0x0F) << 8) | b[3]);

  // FT6336G reports portrait coordinates. TFT rotation=1 is landscape.
  const int16_t x = static_cast<int16_t>(rawY);
  const int16_t y = static_cast<int16_t>(239 - rawX);
  if (x < 0 || x >= 320 || y < 0 || y >= 240) return p;

  p.rawX = rawX;
  p.rawY = rawY;
  p.x = x;
  p.y = y;
  p.touched = true;
  return p;
}

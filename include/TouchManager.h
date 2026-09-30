#pragma once
#include <Arduino.h>

struct TouchPoint {
  bool touched = false;
  int16_t x = 0;
  int16_t y = 0;
  uint16_t rawX = 0;
  uint16_t rawY = 0;
};

class TouchManager {
public:
  void begin();
  TouchPoint read();

private:
  bool readBytes(uint8_t reg, uint8_t* data, size_t len);
};

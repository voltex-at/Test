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
  uint16_t read12(uint8_t command);
  uint8_t transfer8(uint8_t value);
};

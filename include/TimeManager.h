#pragma once
#include <Arduino.h>
#include <time.h>

class TimeManager {
public:
  void begin();
  bool sync(uint32_t timeoutMs);
  bool getLocal(tm& out) const;
  bool isValid() const;
};

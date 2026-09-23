#include "TimeManager.h"
#include "Config.h"

void TimeManager::begin() {
  configTzTime(AppConfig::TZ_INFO, AppConfig::NTP_1, AppConfig::NTP_2, AppConfig::NTP_3);
}

bool TimeManager::sync(uint32_t timeoutMs) {
  const uint32_t start = millis();
  tm t{};
  while (millis() - start < timeoutMs) {
    if (getLocalTime(&t, 300)) {
      const int year = t.tm_year + 1900;
      if (year >= 2024) return true;
    }
    delay(100);
  }
  return false;
}

bool TimeManager::getLocal(tm& out) const {
  if (!getLocalTime(&out, 50)) return false;
  return (out.tm_year + 1900) >= 2024;
}

bool TimeManager::isValid() const {
  tm t{};
  return getLocal(t);
}

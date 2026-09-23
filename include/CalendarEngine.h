#pragma once
#include <Arduino.h>
#include <time.h>
#include "SettingsManager.h"

enum class SceneType : uint8_t {
  NORMAL,
  KRAMPUS,
  NIKOLAUS,
  CHRISTMAS,
  GRINCH,
  SILVESTER,
  BIRTHDAY
};

struct CalendarState {
  SceneType scene = SceneType::NORMAL;
  int daysToChristmas = 0;
  int targetYear = 0;
  bool christmasGreeting = false;
  bool birthday = false;
};

class CalendarEngine {
public:
  CalendarState evaluate(const tm& localTime, const DeviceSettings& settings) const;
  static const char* sceneImage(SceneType scene);
  static const char* sceneLabel(SceneType scene);

private:
  static int64_t daysFromCivil(int year, unsigned month, unsigned day);
};

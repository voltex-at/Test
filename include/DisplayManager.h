#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "CalendarEngine.h"
#include "SettingsManager.h"

class DisplayManager {
public:
  bool begin();
  bool beginSd();
  bool sdReady() const { return sdReady_; }
  void showBoot(const String& text);
  void showWifiStatus(const String& title, const String& line1, const String& line2 = "");
  void showCalendar(const CalendarState& state, const DeviceSettings& settings, const tm& localTime);
  void showTimeError();
  void setBrightness(uint8_t percent);

private:
  TFT_eSPI tft_;
  bool sdReady_ = false;

  bool drawJpg(const char* path, const char* fallback = nullptr);
  void drawCountdownPanel(int days);
  void drawChristmasGreeting();
  void drawBirthdayHeader(const String& name);
  void drawSceneTag(SceneType scene);
  static bool jpgOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
  static TFT_eSPI* callbackTft_;
};

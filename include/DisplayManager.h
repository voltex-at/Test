#pragma once
#include <Arduino.h>
#include "RawIli9341.h"
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
  RawIli9341 tft_;
  bool sdReady_ = false;

  bool drawJpg(const char* path, const char* fallback = nullptr);
  void drawCountdownPanel(int days);
  void drawChristmasGreeting();
  void drawBirthdayHeader(const String& name);
  void drawSceneTag(SceneType scene);
  void drawPlayButton();

  void drawCentered(const String& text, int16_t y, uint8_t size, uint16_t fg, uint16_t bg, bool opaque = true);
  void drawCenteredNumber(int value, int16_t y, uint8_t size, uint16_t fg, uint16_t bg);
  static bool jpgOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
  static RawIli9341* callbackTft_;
};

#include "DisplayManager.h"
#include "Config.h"
#include <SD_MMC.h>
#include <TJpg_Decoder.h>

namespace {
constexpr uint16_t C_BLACK     = 0x0000;
constexpr uint16_t C_NAVY      = 0x000F;
constexpr uint16_t C_DARKGREY  = 0x7BEF;
constexpr uint16_t C_WHITE     = 0xFFFF;
constexpr uint16_t C_YELLOW    = 0xFFE0;
constexpr uint16_t C_GOLD      = 0xFEA0;
constexpr uint16_t C_GREEN     = 0x07E0;
constexpr uint16_t C_LIGHTGREY = 0xD69A;
}

RawIli9341* DisplayManager::callbackTft_ = nullptr;

bool DisplayManager::jpgOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (!callbackTft_) return false;
  if (y >= callbackTft_->height()) return false;
  callbackTft_->pushImage(x, y, w, h, bitmap);
  return true;
}

bool DisplayManager::begin() {
  if (!tft_.begin()) return false;
  callbackTft_ = &tft_;
  TJpgDec.setSwapBytes(false);
  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(jpgOutput);
  return true;
}

bool DisplayManager::beginSd() {
  if (!SD_MMC.setPins(AppConfig::SD_CLK, AppConfig::SD_CMD, AppConfig::SD_D0,
                      AppConfig::SD_D1, AppConfig::SD_D2, AppConfig::SD_D3)) {
    sdReady_ = false;
    return false;
  }
  sdReady_ = SD_MMC.begin("/sdcard", false, false);
  return sdReady_;
}

void DisplayManager::setBrightness(uint8_t percent) {
  tft_.setBacklight(percent != 0);
}

void DisplayManager::drawCentered(const String& text, int16_t y, uint8_t size,
                                  uint16_t fg, uint16_t bg, bool opaque) {
  tft_.setTextSize(size);
  tft_.setTextColor(fg, bg);
  int16_t x1, y1;
  uint16_t w, h;
  tft_.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  const int16_t x = (320 - static_cast<int16_t>(w)) / 2;
  if (opaque) tft_.fillRect(x - 4, y - 3, w + 8, h + 6, bg);
  tft_.setCursor(x, y);
  tft_.print(text);
}

void DisplayManager::drawCenteredNumber(int value, int16_t y, uint8_t size,
                                        uint16_t fg, uint16_t bg) {
  drawCentered(String(value), y, size, fg, bg, true);
}

void DisplayManager::showBoot(const String& text) {
  tft_.fillScreen(C_NAVY);
  drawCentered("WEIHNACHTSUHR", 78, 3, C_WHITE, C_NAVY);
  drawCentered(text, 125, 2, C_GOLD, C_NAVY);
}

bool DisplayManager::drawJpg(const char* path, const char* fallback) {
  if (!sdReady_) return false;
  if (SD_MMC.exists(path)) {
    TJpgDec.drawFsJpg(0, 0, path, SD_MMC);
    return true;
  }
  if (fallback && SD_MMC.exists(fallback)) {
    TJpgDec.drawFsJpg(0, 0, fallback, SD_MMC);
    return true;
  }
  return false;
}

void DisplayManager::showWifiStatus(const String& title, const String& line1, const String& line2) {
  if (!drawJpg("/images/board.jpg", "/images/wifi.jpg")) {
    tft_.fillScreen(C_DARKGREY);
  }
  drawCentered(title, 62, 3, C_WHITE, C_DARKGREY);
  drawCentered(line1, 110, 2, C_YELLOW, C_DARKGREY);
  if (line2.length()) drawCentered(line2, 145, 2, C_WHITE, C_DARKGREY);
}

void DisplayManager::drawCountdownPanel(int days) {
  tft_.fillRoundRect(74, 55, 172, 122, 12, C_BLACK);
  tft_.drawRoundRect(74, 55, 172, 122, 12, C_GOLD);

  drawCentered("NOCH", 67, 2, C_WHITE, C_BLACK);
  drawCenteredNumber(days, 96, 5, C_GOLD, C_BLACK);
  drawCentered(days == 1 ? "TAG" : "TAGE", 145, 2, C_WHITE, C_BLACK);
  drawCentered("BIS WEIHNACHTEN", 166, 1, C_WHITE, C_BLACK);
}

void DisplayManager::drawChristmasGreeting() {
  tft_.fillRoundRect(34, 66, 252, 104, 14, C_BLACK);
  tft_.drawRoundRect(34, 66, 252, 104, 14, C_GOLD);
  drawCentered("FROHE", 83, 3, C_GOLD, C_BLACK);
  drawCentered("WEIHNACHTEN", 121, 2, C_WHITE, C_BLACK);
  drawCentered("24 - 26 DEZEMBER", 151, 1, C_GREEN, C_BLACK);
}

void DisplayManager::drawBirthdayHeader(const String& name) {
  tft_.fillRoundRect(34, 7, 252, 38, 10, C_BLACK);
  String text = "HAPPY BIRTHDAY";
  if (name.length()) text += " " + name;
  if (text.length() > 24) text = "HAPPY BIRTHDAY!";
  drawCentered(text, 18, 1, C_GOLD, C_BLACK);
}

void DisplayManager::drawSceneTag(SceneType scene) {
  if (scene == SceneType::NORMAL || scene == SceneType::BIRTHDAY || scene == SceneType::CHRISTMAS) return;
  const char* label = CalendarEngine::sceneLabel(scene);
  if (!label || !*label) return;
  tft_.fillRoundRect(8, 8, 110, 28, 7, C_BLACK);
  tft_.setTextSize(1);
  tft_.setTextColor(C_WHITE, C_BLACK);
  tft_.setCursor(16, 18);
  tft_.print(label);
}

void DisplayManager::drawPlayButton() {
  if (!AppConfig::AUDIO_FEATURE_ENABLED) return;
  const int16_t cx = 292;
  const int16_t cy = 27;
  tft_.fillCircle(cx, cy, 20, C_BLACK);
  tft_.drawCircle(cx, cy, 20, C_GOLD);
  tft_.drawCircle(cx, cy, 19, C_GOLD);
  tft_.fillTriangle(cx - 5, cy - 9, cx - 5, cy + 9, cx + 10, cy, C_WHITE);
}

void DisplayManager::showCalendar(const CalendarState& state, const DeviceSettings& settings, const tm& localTime) {
  const char* image = CalendarEngine::sceneImage(state.scene);
  if (!drawJpg(image, "/images/normal.jpg")) {
    tft_.fillScreen(C_NAVY);
  }

  if (state.christmasGreeting) {
    drawChristmasGreeting();
    drawPlayButton();
    return;
  }

  if (state.birthday) drawBirthdayHeader(settings.birthdayName);
  else drawSceneTag(state.scene);

  drawCountdownPanel(state.daysToChristmas);

  char dateBuf[20];
  snprintf(dateBuf, sizeof(dateBuf), "%02d.%02d.%04d",
           localTime.tm_mday, localTime.tm_mon + 1, localTime.tm_year + 1900);
  tft_.fillRoundRect(100, 211, 120, 22, 6, C_BLACK);
  drawCentered(String(dateBuf), 217, 1, C_LIGHTGREY, C_BLACK);

  drawPlayButton();
}

void DisplayManager::showTimeError() {
  showWifiStatus("ZEIT FEHLT", "NTP wird erneut versucht", "WLAN pruefen");
}

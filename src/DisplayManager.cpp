#include "DisplayManager.h"
#include "Config.h"
#include <SD_MMC.h>
#include <TJpg_Decoder.h>

TFT_eSPI* DisplayManager::callbackTft_ = nullptr;

bool DisplayManager::jpgOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (!callbackTft_) return false;
  if (y >= callbackTft_->height()) return false;
  callbackTft_->pushImage(x, y, w, h, bitmap);
  return true;
}

bool DisplayManager::begin() {
  pinMode(AppConfig::TFT_BACKLIGHT_PIN, OUTPUT);
  digitalWrite(AppConfig::TFT_BACKLIGHT_PIN, HIGH);

  tft_.init();

  // ES3C28P uses an ILI9341V IPS panel. The manufacturer's demo uses a
  // slightly different init path than generic ILI9341 modules. Force the
  // exact command sequence that already worked on this physical board.
  tft_.writecommand(0x01); delay(120); // SWRESET
  tft_.writecommand(0x11); delay(120); // SLPOUT
  tft_.writecommand(0x3A); tft_.writedata(0x55); // RGB565
  tft_.writecommand(0x36); tft_.writedata(0x28); // landscape + BGR
  tft_.writecommand(0x21);                         // inversion ON for IPS
  tft_.writecommand(0x29); delay(20);              // display ON

  tft_.setRotation(AppConfig::TFT_ROTATION);
  tft_.invertDisplay(true);
  digitalWrite(AppConfig::TFT_BACKLIGHT_PIN, HIGH);
  tft_.fillScreen(TFT_BLACK);
  tft_.setTextWrap(false);

  callbackTft_ = &tft_;
  TJpgDec.setSwapBytes(true);
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
  digitalWrite(AppConfig::TFT_BACKLIGHT_PIN, percent == 0 ? LOW : HIGH);
}

void DisplayManager::showBoot(const String& text) {
  tft_.fillScreen(TFT_NAVY);
  tft_.setTextDatum(MC_DATUM);
  tft_.setTextColor(TFT_WHITE, TFT_NAVY);
  tft_.drawString("WEIHNACHTSUHR", 160, 88, 4);
  tft_.setTextColor(TFT_GOLD, TFT_NAVY);
  tft_.drawString(text, 160, 135, 2);
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
    tft_.fillScreen(TFT_DARKGREY);
  }

  tft_.setTextDatum(MC_DATUM);
  tft_.setTextColor(TFT_WHITE);
  tft_.drawString(title, 190, 78, 4);
  tft_.setTextColor(TFT_YELLOW);
  tft_.drawString(line1, 190, 118, 2);
  if (line2.length()) {
    tft_.setTextColor(TFT_WHITE);
    tft_.drawString(line2, 190, 145, 2);
  }
}

void DisplayManager::drawCountdownPanel(int days) {
  tft_.fillRoundRect(74, 55, 172, 122, 12, TFT_BLACK);
  tft_.drawRoundRect(74, 55, 172, 122, 12, TFT_GOLD);
  tft_.setTextDatum(MC_DATUM);
  tft_.setTextColor(TFT_WHITE, TFT_BLACK);
  tft_.drawString("NOCH", 160, 76, 4);

  tft_.setTextColor(TFT_GOLD, TFT_BLACK);
  tft_.drawNumber(days, 160, 119, 7);

  tft_.setTextColor(TFT_WHITE, TFT_BLACK);
  tft_.drawString(days == 1 ? "TAG" : "TAGE", 160, 151, 4);
  tft_.drawString("BIS WEIHNACHTEN", 160, 169, 2);
}

void DisplayManager::drawChristmasGreeting() {
  tft_.fillRoundRect(34, 66, 252, 104, 14, TFT_BLACK);
  tft_.drawRoundRect(34, 66, 252, 104, 14, TFT_GOLD);
  tft_.setTextDatum(MC_DATUM);
  tft_.setTextColor(TFT_GOLD, TFT_BLACK);
  tft_.drawString("FROHE", 160, 94, 4);
  tft_.setTextColor(TFT_WHITE, TFT_BLACK);
  tft_.drawString("WEIHNACHTEN", 160, 132, 4);
  tft_.setTextColor(TFT_GREEN, TFT_BLACK);
  tft_.drawString("24 - 26 DEZEMBER", 160, 158, 2);
}

void DisplayManager::drawBirthdayHeader(const String& name) {
  tft_.fillRoundRect(64, 7, 192, 38, 10, TFT_BLACK);
  tft_.setTextDatum(MC_DATUM);
  tft_.setTextColor(TFT_GOLD, TFT_BLACK);
  String text = "HAPPY BIRTHDAY";
  if (name.length()) text += " " + name;
  if (text.length() > 24) text = "HAPPY BIRTHDAY!";
  tft_.drawString(text, 160, 26, 2);
}

void DisplayManager::drawSceneTag(SceneType scene) {
  if (scene == SceneType::NORMAL || scene == SceneType::BIRTHDAY || scene == SceneType::CHRISTMAS) return;
  const char* label = CalendarEngine::sceneLabel(scene);
  if (!label || !*label) return;
  tft_.fillRoundRect(8, 8, 110, 28, 7, TFT_BLACK);
  tft_.setTextDatum(MC_DATUM);
  tft_.setTextColor(TFT_WHITE, TFT_BLACK);
  tft_.drawString(label, 63, 22, 2);
}

void DisplayManager::drawPlayButton() {
  if (!AppConfig::AUDIO_FEATURE_ENABLED) return;

  const int16_t cx = 292;
  const int16_t cy = 27;
  tft_.fillCircle(cx, cy, 20, TFT_BLACK);
  tft_.drawCircle(cx, cy, 20, TFT_GOLD);
  tft_.drawCircle(cx, cy, 19, TFT_GOLD);
  tft_.fillTriangle(cx - 5, cy - 9, cx - 5, cy + 9, cx + 10, cy, TFT_WHITE);
}

void DisplayManager::showCalendar(const CalendarState& state, const DeviceSettings& settings, const tm& localTime) {
  const char* image = CalendarEngine::sceneImage(state.scene);
  if (!drawJpg(image, "/images/normal.jpg")) {
    tft_.fillScreen(TFT_NAVY);
  }

  if (state.christmasGreeting) {
    drawChristmasGreeting();
    drawPlayButton();
    return;
  }

  if (state.birthday) {
    drawBirthdayHeader(settings.birthdayName);
  } else {
    drawSceneTag(state.scene);
  }

  drawCountdownPanel(state.daysToChristmas);

  char dateBuf[20];
  snprintf(dateBuf, sizeof(dateBuf), "%02d.%02d.%04d", localTime.tm_mday, localTime.tm_mon + 1, localTime.tm_year + 1900);
  tft_.fillRoundRect(100, 211, 120, 22, 6, TFT_BLACK);
  tft_.setTextDatum(MC_DATUM);
  tft_.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft_.drawString(dateBuf, 160, 222, 2);

  drawPlayButton();
}

void DisplayManager::showTimeError() {
  showWifiStatus("ZEIT FEHLT", "NTP wird erneut versucht", "WLAN pruefen");
}

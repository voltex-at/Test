#pragma once
#include <Arduino.h>

namespace AppConfig {
  constexpr char DEVICE_NAME[] = "Weihnachtsuhr";
  constexpr char AP_SSID[] = "Weihnachtsuhr-Setup";
  constexpr char AP_PASSWORD[] = ""; // otwarta siec konfiguracyjna

  // Europa Centralna: CET/CEST z automatyczna zmiana czasu.
  constexpr char TZ_INFO[] = "CET-1CEST,M3.5.0,M10.5.0/3";
  constexpr char NTP_1[] = "pool.ntp.org";
  constexpr char NTP_2[] = "time.cloudflare.com";
  constexpr char NTP_3[] = "time.google.com";

  constexpr uint8_t TFT_ROTATION = 1; // 320x240 landscape
  constexpr uint8_t TFT_BACKLIGHT_PIN = 21;

  // microSD - osobny sprzetowy SPI (VSPI)
  constexpr uint8_t SD_CS   = 5;
  constexpr uint8_t SD_SCK  = 18;
  constexpr uint8_t SD_MISO = 19;
  constexpr uint8_t SD_MOSI = 23;

  // XPT2046 - na CYD ma osobne piny. Uzywamy bit-bang SPI,
  // aby nie konfliktowac jednoczesnie z TFT i karta SD.
  constexpr uint8_t TOUCH_CLK  = 25;
  constexpr uint8_t TOUCH_MOSI = 32;
  constexpr uint8_t TOUCH_MISO = 39;
  constexpr uint8_t TOUCH_CS   = 33;
  constexpr uint8_t TOUCH_IRQ  = 36;

  constexpr int TOUCH_X_MIN = 180;
  constexpr int TOUCH_X_MAX = 3800;
  constexpr int TOUCH_Y_MIN = 240;
  constexpr int TOUCH_Y_MAX = 3860;

  // Audio dopasowane do tej rewizji CYD:
  // DAC2/GPIO26 -> wbudowany wzmacniacz. Testy pokazaly, ze ok. +/-8
  // wokol 128 daje najlepszy kompromis glosnosc/znieksztalcenia.
  constexpr uint8_t AUDIO_DAC_PIN = 26;
  constexpr uint8_t AUDIO_DAC_AMPLITUDE = 8;
  constexpr bool AUDIO_FEATURE_ENABLED = true;

  // Pole dotykowe przycisku Play w prawym gornym rogu.
  constexpr int16_t PLAY_X1 = 264;
  constexpr int16_t PLAY_Y1 = 0;
  constexpr int16_t PLAY_X2 = 319;
  constexpr int16_t PLAY_Y2 = 56;

  constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 18000;
  constexpr uint32_t NTP_SYNC_TIMEOUT_MS = 15000;
  constexpr uint32_t SCREEN_REFRESH_MS = 30000;
}

#pragma once
#include <Arduino.h>

namespace AppConfig {
  constexpr char DEVICE_NAME[] = "Weihnachtsuhr";
  constexpr char AP_SSID[] = "Weihnachtsuhr-Setup";
  constexpr char AP_PASSWORD[] = "";

  constexpr char TZ_INFO[] = "CET-1CEST,M3.5.0,M10.5.0/3";
  constexpr char NTP_1[] = "pool.ntp.org";
  constexpr char NTP_2[] = "time.cloudflare.com";
  constexpr char NTP_3[] = "time.google.com";

  // ES3C28P / ESP32-S3, ILI9341V 240x320, landscape 320x240
  constexpr uint8_t TFT_ROTATION = 1;
  constexpr uint8_t TFT_BACKLIGHT_PIN = 45;

  // Shared I2C: FT6336G touch + ES8311 codec
  constexpr uint8_t I2C_SDA = 16;
  constexpr uint8_t I2C_SCL = 15;

  // FT6336G capacitive touch
  constexpr uint8_t TOUCH_RST = 18;
  constexpr uint8_t TOUCH_IRQ = 17;
  constexpr uint8_t TOUCH_ADDR = 0x38;

  // microSD in native 4-bit SD_MMC mode
  constexpr uint8_t SD_CLK = 38;
  constexpr uint8_t SD_CMD = 40;
  constexpr uint8_t SD_D0  = 39;
  constexpr uint8_t SD_D1  = 41;
  constexpr uint8_t SD_D2  = 48;
  constexpr uint8_t SD_D3  = 47;

  // ES8311 I2S + FM8002E amplifier
  constexpr uint8_t AUDIO_AMP_EN = 1; // active LOW
  constexpr uint8_t AUDIO_MCLK = 4;
  constexpr uint8_t AUDIO_BCLK = 5;
  constexpr uint8_t AUDIO_DOUT = 6;  // ESP32-S3 -> ES8311 / speaker path
  constexpr uint8_t AUDIO_LRCK = 7;
  constexpr uint8_t AUDIO_DIN  = 8;  // ES8311 -> ESP32-S3 / microphone path
  constexpr uint8_t ES8311_ADDR = 0x18;
  constexpr bool AUDIO_FEATURE_ENABLED = true;

  constexpr int16_t PLAY_X1 = 264;
  constexpr int16_t PLAY_Y1 = 0;
  constexpr int16_t PLAY_X2 = 319;
  constexpr int16_t PLAY_Y2 = 56;

  constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 18000;
  constexpr uint32_t NTP_SYNC_TIMEOUT_MS = 15000;
  constexpr uint32_t SCREEN_REFRESH_MS = 30000;
}

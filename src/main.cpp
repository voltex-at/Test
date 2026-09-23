#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "Config.h"
#include "SettingsManager.h"
#include "CalendarEngine.h"
#include "TimeManager.h"
#include "WifiManager.h"
#include "WebPortal.h"
#include "DisplayManager.h"
#include "TouchManager.h"
#include "AudioManager.h"

SettingsManager settingsManager;
CalendarEngine calendarEngine;
TimeManager timeManager;
WifiManager wifiManager;
DisplayManager displayManager;
TouchManager touchManager;
AudioManager audioManager;
WebPortal webPortal(settingsManager, wifiManager);

DeviceSettings settings;
CalendarState currentCalendarState;
bool calendarStateReady = false;
bool timeReady = false;
bool touchLatched = false;
uint32_t lastScreenRefresh = 0;
uint32_t lastNtpRetry = 0;
uint32_t restartAt = 0;

static void setupOta() {
  ArduinoOTA.setHostname("weihnachtsuhr");
  ArduinoOTA.setPassword("weihnachten");
  ArduinoOTA.begin();
}

static void refreshCalendar(bool force = false) {
  if (!timeReady) return;
  if (!force && millis() - lastScreenRefresh < AppConfig::SCREEN_REFRESH_MS) return;

  tm local{};
  if (!timeManager.getLocal(local)) {
    timeReady = false;
    calendarStateReady = false;
    displayManager.showTimeError();
    return;
  }

  settings = settingsManager.load();
  currentCalendarState = calendarEngine.evaluate(local, settings);
  calendarStateReady = true;
  displayManager.showCalendar(currentCalendarState, settings, local);
  lastScreenRefresh = millis();
}

static bool inPlayButton(const TouchPoint& p) {
  return p.x >= AppConfig::PLAY_X1 && p.x <= AppConfig::PLAY_X2 &&
         p.y >= AppConfig::PLAY_Y1 && p.y <= AppConfig::PLAY_Y2;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Najwazniejsze dla tej rewizji CYD: audio od bootu ma byc fizycznie odlaczone.
  audioManager.begin();

  settingsManager.begin();
  settings = settingsManager.load();

  displayManager.begin();
  displayManager.showBoot("Start...");
  delay(300);

  const bool sdOk = displayManager.beginSd();
  Serial.printf("SD: %s\n", sdOk ? "OK" : "FEHLT");

  touchManager.begin();

  if (settingsManager.hasWifi()) {
    displayManager.showWifiStatus("WLAN", "Verbinde...", settings.ssid);
    if (wifiManager.connectStation(settings, AppConfig::WIFI_CONNECT_TIMEOUT_MS)) {
      displayManager.showWifiStatus("WLAN VERBUNDEN", WiFi.SSID(), wifiManager.ip());
      delay(1000);

      timeManager.begin();
      displayManager.showWifiStatus("ZEIT", "NTP Synchronisation...", "Bitte warten");
      timeReady = timeManager.sync(AppConfig::NTP_SYNC_TIMEOUT_MS);
      if (!timeReady) displayManager.showTimeError();

      setupOta();
    } else {
      wifiManager.startSetupAp();
      displayManager.showWifiStatus("WLAN SETUP", AppConfig::AP_SSID, wifiManager.ip());
    }
  } else {
    wifiManager.startSetupAp();
    displayManager.showWifiStatus("WLAN SETUP", AppConfig::AP_SSID, wifiManager.ip());
  }

  webPortal.begin();
  if (timeReady) refreshCalendar(true);
}

void loop() {
  wifiManager.processDns();
  webPortal.handle();

  if (wifiManager.connected()) {
    ArduinoOTA.handle();
  }

  if (webPortal.restartRequested() && restartAt == 0) {
    restartAt = millis() + 1200;
  }
  if (restartAt && static_cast<int32_t>(millis() - restartAt) >= 0) {
    audioManager.stop();
    ESP.restart();
  }

  if (!timeReady && wifiManager.connected() && millis() - lastNtpRetry > 30000) {
    lastNtpRetry = millis();
    timeManager.begin();
    timeReady = timeManager.sync(5000);
    if (timeReady) refreshCalendar(true);
  }

  refreshCalendar(false);

  if (AppConfig::AUDIO_FEATURE_ENABLED) {
    const TouchPoint p = touchManager.read();

    if (!p.touched) {
      touchLatched = false;
    } else if (!touchLatched) {
      touchLatched = true;

      if (inPlayButton(p)) {
        const SceneType scene = calendarStateReady ? currentCalendarState.scene : SceneType::NORMAL;
        Serial.printf("Audio Play: scene=%u\n", static_cast<unsigned>(scene));

        audioManager.playForScene(scene);

        // Po melodii DAC jest juz OFF/Hi-Z. Odswiezamy ekran po blokujacym odtwarzaniu.
        if (timeReady) refreshCalendar(true);
      }
    }
  }

  delay(5);
}

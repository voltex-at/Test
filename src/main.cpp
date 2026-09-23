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

SettingsManager settingsManager;
CalendarEngine calendarEngine;
TimeManager timeManager;
WifiManager wifiManager;
DisplayManager displayManager;
TouchManager touchManager;
WebPortal webPortal(settingsManager, wifiManager);

DeviceSettings settings;
bool timeReady = false;
uint32_t lastScreenRefresh = 0;
uint32_t lastNtpRetry = 0;
uint32_t restartAt = 0;

static void setupOta() {
  ArduinoOTA.setHostname("weihnachtsuhr");
  ArduinoOTA.setPassword("weihnachten"); // zmien przed udostepnieniem urzadzenia poza domowa siecia
  ArduinoOTA.begin();
}

static void refreshCalendar(bool force = false) {
  if (!timeReady) return;
  if (!force && millis() - lastScreenRefresh < AppConfig::SCREEN_REFRESH_MS) return;

  tm local{};
  if (!timeManager.getLocal(local)) {
    timeReady = false;
    displayManager.showTimeError();
    return;
  }

  settings = settingsManager.load();
  const CalendarState state = calendarEngine.evaluate(local, settings);
  displayManager.showCalendar(state, settings, local);
  lastScreenRefresh = millis();
}

void setup() {
  Serial.begin(115200);
  delay(250);

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
    ESP.restart();
  }

  if (!timeReady && wifiManager.connected() && millis() - lastNtpRetry > 30000) {
    lastNtpRetry = millis();
    timeManager.begin();
    timeReady = timeManager.sync(5000);
    if (timeReady) refreshCalendar(true);
  }

  refreshCalendar(false);

  // Touch jest od poczatku inicjalizowany. Przycisk Play dodamy w v2 razem z audio.
  if (AppConfig::AUDIO_FEATURE_ENABLED) {
    TouchPoint p = touchManager.read();
    if (p.touched) {
      Serial.printf("Touch: %d,%d raw=%u,%u\n", p.x, p.y, p.rawX, p.rawY);
    }
  }

  delay(5);
}

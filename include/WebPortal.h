#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include "SettingsManager.h"
#include "WifiManager.h"

class WebPortal {
public:
  WebPortal(SettingsManager& settings, WifiManager& wifi);
  void begin();
  void handle();
  bool restartRequested() const { return restartRequested_; }

private:
  WebServer server_{80};
  SettingsManager& settings_;
  WifiManager& wifi_;
  bool restartRequested_ = false;

  void handleRoot();
  void handleSave();
  void handleResetWifi();
  void handleNotFound();
  String pageHtml(const DeviceSettings& s);
  static String htmlEscape(const String& value);
  static bool parseBirthday(const String& value, uint8_t& day, uint8_t& month);
};

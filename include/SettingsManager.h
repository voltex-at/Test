#pragma once
#include <Arduino.h>
#include <Preferences.h>

struct DeviceSettings {
  String ssid;
  String password;
  String birthdayName;
  uint8_t birthdayDay = 0;
  uint8_t birthdayMonth = 0;
};

class SettingsManager {
public:
  void begin();
  DeviceSettings load();
  void saveWifi(const String& ssid, const String& password);
  void saveBirthday(const String& name, uint8_t day, uint8_t month);
  void clearWifi();
  bool hasWifi() const;

private:
  Preferences prefs_;
  DeviceSettings settings_;
};

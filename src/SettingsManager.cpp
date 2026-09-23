#include "SettingsManager.h"

void SettingsManager::begin() {
  prefs_.begin("xmasclock", false);
  settings_ = load();
}

DeviceSettings SettingsManager::load() {
  DeviceSettings s;
  s.ssid = prefs_.getString("ssid", "");
  s.password = prefs_.getString("pass", "");
  s.birthdayName = prefs_.getString("bname", "");
  s.birthdayDay = prefs_.getUChar("bday", 0);
  s.birthdayMonth = prefs_.getUChar("bmonth", 0);
  settings_ = s;
  return s;
}

void SettingsManager::saveWifi(const String& ssid, const String& password) {
  prefs_.putString("ssid", ssid);
  prefs_.putString("pass", password);
  settings_.ssid = ssid;
  settings_.password = password;
}

void SettingsManager::saveBirthday(const String& name, uint8_t day, uint8_t month) {
  prefs_.putString("bname", name);
  prefs_.putUChar("bday", day);
  prefs_.putUChar("bmonth", month);
  settings_.birthdayName = name;
  settings_.birthdayDay = day;
  settings_.birthdayMonth = month;
}

void SettingsManager::clearWifi() {
  prefs_.remove("ssid");
  prefs_.remove("pass");
  settings_.ssid = "";
  settings_.password = "";
}

bool SettingsManager::hasWifi() const {
  return settings_.ssid.length() > 0;
}

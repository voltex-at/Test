#include "WifiManager.h"
#include "Config.h"

bool WifiManager::connectStation(const DeviceSettings& settings, uint32_t timeoutMs) {
  if (settings.ssid.isEmpty()) return false;

  WiFi.mode(WIFI_STA);
  WiFi.setHostname("weihnachtsuhr");
  WiFi.begin(settings.ssid.c_str(), settings.password.c_str());

  const uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (WiFi.status() == WL_CONNECTED) return true;
    delay(200);
  }
  return false;
}

void WifiManager::startSetupAp() {
  WiFi.disconnect(true, true);
  delay(100);
  WiFi.mode(WIFI_AP_STA);
  if (strlen(AppConfig::AP_PASSWORD) >= 8) {
    WiFi.softAP(AppConfig::AP_SSID, AppConfig::AP_PASSWORD);
  } else {
    WiFi.softAP(AppConfig::AP_SSID);
  }
  dns_.start(53, "*", WiFi.softAPIP());
  apMode_ = true;
}

void WifiManager::stopSetupAp() {
  if (!apMode_) return;
  dns_.stop();
  WiFi.softAPdisconnect(true);
  apMode_ = false;
}

void WifiManager::processDns() {
  if (apMode_) dns_.processNextRequest();
}

String WifiManager::ip() const {
  if (apMode_) return WiFi.softAPIP().toString();
  if (connected()) return WiFi.localIP().toString();
  return "0.0.0.0";
}

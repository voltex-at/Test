#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "SettingsManager.h"

class WifiManager {
public:
  bool connectStation(const DeviceSettings& settings, uint32_t timeoutMs);
  void startSetupAp();
  void stopSetupAp();
  void processDns();
  bool apMode() const { return apMode_; }
  bool connected() const { return WiFi.status() == WL_CONNECTED; }
  String ip() const;

private:
  bool apMode_ = false;
  DNSServer dns_;
};

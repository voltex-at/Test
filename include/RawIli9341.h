#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include "Config.h"

class RawIli9341 : public Adafruit_GFX {
public:
  RawIli9341();

  bool begin();
  void setBacklight(bool on);
  void pushImage(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t* pixels);

  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void startWrite() override {}
  void endWrite() override {}
  void writePixel(int16_t x, int16_t y, uint16_t color) override;
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;

private:
  SPIClass spi_{FSPI};

  void command(uint8_t cmd);
  void data8(uint8_t data);
  void setAddrWindowRaw(int16_t x, int16_t y, int16_t w, int16_t h);
  void pushColor(uint16_t color, uint32_t count);
};

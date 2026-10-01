#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
constexpr uint8_t TL_DATUM = 0;
constexpr uint8_t MC_DATUM = 1;
constexpr uint8_t TC_DATUM = 2;
class PeppiDisplay : public Adafruit_GFX {
public:
  static constexpr int16_t WIDTH = 320;
  static constexpr int16_t HEIGHT = 240;
  PeppiDisplay();
  void init();
  void setRotation(uint8_t r);
  void setBacklight(uint8_t value);
  void setSwapBytes(bool) {}
  void setTextDatum(uint8_t datum) { datum_ = datum; }
  void setFreeFont(const GFXfont* font) { setFont(font); }
  void setTextFont(uint8_t font);
  int16_t textWidth(const String& value);
  int16_t textWidth(const String& value, int font);
  int16_t drawString(const String& value, int32_t x, int32_t y, int font = 0);
  void pushImage(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t* pixels);
  void drawEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color);
  void fillEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color);
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void startWrite() override {}
  void endWrite() override {}
  void writePixel(int16_t x, int16_t y, uint16_t color) override;
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
private:
  SPIClass spi_{FSPI};
  uint8_t datum_ = TL_DATUM;
  static constexpr uint8_t PIN_CS = 10;
  static constexpr uint8_t PIN_DC = 46;
  static constexpr uint8_t PIN_SCK = 12;
  static constexpr uint8_t PIN_MOSI = 11;
  static constexpr uint8_t PIN_MISO = 13;
  static constexpr uint8_t PIN_BL = 45;
  void command(uint8_t cmd);
  void data8(uint8_t data);
  void setAddrWindowRaw(int16_t x, int16_t y, int16_t w, int16_t h);
  void pushColor(uint16_t color, uint32_t count);
  uint8_t scaleForFont(int font) const;
};
using TFT_eSPI = PeppiDisplay;

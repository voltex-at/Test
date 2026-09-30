#include "RawIli9341.h"

RawIli9341::RawIli9341() : Adafruit_GFX(320, 240) {}

void RawIli9341::command(uint8_t cmd) {
  digitalWrite(AppConfig::TFT_DC, LOW);
  digitalWrite(AppConfig::TFT_CS, LOW);
  spi_.transfer(cmd);
  digitalWrite(AppConfig::TFT_CS, HIGH);
}

void RawIli9341::data8(uint8_t data) {
  digitalWrite(AppConfig::TFT_DC, HIGH);
  digitalWrite(AppConfig::TFT_CS, LOW);
  spi_.transfer(data);
  digitalWrite(AppConfig::TFT_CS, HIGH);
}

bool RawIli9341::begin() {
  pinMode(AppConfig::TFT_CS, OUTPUT);
  pinMode(AppConfig::TFT_DC, OUTPUT);
  pinMode(AppConfig::TFT_BACKLIGHT_PIN, OUTPUT);

  digitalWrite(AppConfig::TFT_CS, HIGH);
  digitalWrite(AppConfig::TFT_DC, HIGH);
  digitalWrite(AppConfig::TFT_BACKLIGHT_PIN, LOW);

  // This is intentionally identical to the low-level SPI setup from the
  // first test firmware that produced a correct image on the user's board.
  spi_.begin(AppConfig::TFT_SCK, AppConfig::TFT_MISO,
             AppConfig::TFT_MOSI, AppConfig::TFT_CS);
  spi_.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));

  command(0x01); delay(120);              // SWRESET
  command(0x11); delay(120);              // SLPOUT
  command(0x3A); data8(0x55);             // RGB565
  command(0x36); data8(0x28);             // landscape + BGR
  command(0x21);                          // inversion ON (IPS)
  command(0x29); delay(20);               // DISPON

  digitalWrite(AppConfig::TFT_BACKLIGHT_PIN, HIGH);
  fillScreen(0x0000);
  setTextWrap(false);
  return true;
}

void RawIli9341::setBacklight(bool on) {
  digitalWrite(AppConfig::TFT_BACKLIGHT_PIN, on ? HIGH : LOW);
}

void RawIli9341::setAddrWindowRaw(int16_t x, int16_t y, int16_t w, int16_t h) {
  const int16_t x2 = x + w - 1;
  const int16_t y2 = y + h - 1;

  command(0x2A);
  digitalWrite(AppConfig::TFT_DC, HIGH);
  digitalWrite(AppConfig::TFT_CS, LOW);
  spi_.transfer16(static_cast<uint16_t>(x));
  spi_.transfer16(static_cast<uint16_t>(x2));
  digitalWrite(AppConfig::TFT_CS, HIGH);

  command(0x2B);
  digitalWrite(AppConfig::TFT_DC, HIGH);
  digitalWrite(AppConfig::TFT_CS, LOW);
  spi_.transfer16(static_cast<uint16_t>(y));
  spi_.transfer16(static_cast<uint16_t>(y2));
  digitalWrite(AppConfig::TFT_CS, HIGH);

  command(0x2C);
}

void RawIli9341::pushColor(uint16_t color, uint32_t count) {
  digitalWrite(AppConfig::TFT_DC, HIGH);
  digitalWrite(AppConfig::TFT_CS, LOW);
  while (count--) spi_.transfer16(color);
  digitalWrite(AppConfig::TFT_CS, HIGH);
}

void RawIli9341::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (w <= 0 || h <= 0) return;
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x >= width() || y >= height()) return;
  if (x + w > width()) w = width() - x;
  if (y + h > height()) h = height() - y;
  if (w <= 0 || h <= 0) return;

  setAddrWindowRaw(x, y, w, h);
  pushColor(color, static_cast<uint32_t>(w) * static_cast<uint32_t>(h));
}

void RawIli9341::drawPixel(int16_t x, int16_t y, uint16_t color) {
  writePixel(x, y, color);
}

void RawIli9341::writePixel(int16_t x, int16_t y, uint16_t color) {
  if (x < 0 || y < 0 || x >= width() || y >= height()) return;
  setAddrWindowRaw(x, y, 1, 1);
  pushColor(color, 1);
}

void RawIli9341::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  writeFillRect(x, y, w, 1, color);
}

void RawIli9341::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  writeFillRect(x, y, 1, h, color);
}

void RawIli9341::pushImage(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t* pixels) {
  if (!pixels || w == 0 || h == 0) return;
  if (x < 0 || y < 0 || x + w > width() || y + h > height()) {
    // JPEG blocks are expected in-bounds; fall back safely for edge blocks.
    for (uint16_t yy = 0; yy < h; ++yy) {
      for (uint16_t xx = 0; xx < w; ++xx) {
        drawPixel(x + xx, y + yy, pixels[static_cast<uint32_t>(yy) * w + xx]);
      }
    }
    return;
  }

  setAddrWindowRaw(x, y, w, h);
  digitalWrite(AppConfig::TFT_DC, HIGH);
  digitalWrite(AppConfig::TFT_CS, LOW);
  const uint32_t n = static_cast<uint32_t>(w) * h;
  for (uint32_t i = 0; i < n; ++i) spi_.transfer16(pixels[i]);
  digitalWrite(AppConfig::TFT_CS, HIGH);
}

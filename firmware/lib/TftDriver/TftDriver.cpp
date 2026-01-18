#include "TftDriver.h"

// Hardware SPI constructor (uses default MOSI=23, SCK=18)
TftDriver::TftDriver(int8_t cs, int8_t dc, int8_t rst)
    : _tft(cs, dc, rst) {
}

// Software SPI constructor (custom pins for all SPI signals)
TftDriver::TftDriver(int8_t cs, int8_t dc, int8_t mosi, int8_t sck, int8_t rst)
    : _tft(cs, dc, mosi, sck, rst) {
}

void TftDriver::init(uint8_t tabColor) {
    _tft.initR(tabColor);
    _tft.fillScreen(ST77XX_BLACK);
}

void TftDriver::setRotation(uint8_t rotation) {
    _tft.setRotation(rotation);
}

void TftDriver::invertDisplay(bool invert) {
    _tft.invertDisplay(invert);
}

int16_t TftDriver::width() {
    return _tft.width();
}

int16_t TftDriver::height() {
    return _tft.height();
}

// ==================== XOA VA TO MAU ====================

void TftDriver::clear() {
    _tft.fillScreen(ST77XX_BLACK);
}

void TftDriver::fillScreen(uint16_t color) {
    _tft.fillScreen(color);
}

// ==================== VE DIEM VA DUONG ====================

void TftDriver::drawPixel(int16_t x, int16_t y, uint16_t color) {
    _tft.drawPixel(x, y, color);
}

void TftDriver::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    _tft.drawLine(x0, y0, x1, y1, color);
}

void TftDriver::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    _tft.drawFastHLine(x, y, w, color);
}

void TftDriver::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    _tft.drawFastVLine(x, y, h, color);
}

// ==================== HINH CHU NHAT ====================

void TftDriver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    _tft.drawRect(x, y, w, h, color);
}

void TftDriver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    _tft.fillRect(x, y, w, h, color);
}

void TftDriver::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    _tft.drawRoundRect(x, y, w, h, r, color);
}

void TftDriver::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    _tft.fillRoundRect(x, y, w, h, r, color);
}

// ==================== HINH TRON ====================

void TftDriver::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    _tft.drawCircle(x0, y0, r, color);
}

void TftDriver::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    _tft.fillCircle(x0, y0, r, color);
}

// ==================== TAM GIAC ====================

void TftDriver::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
                              int16_t x2, int16_t y2, uint16_t color) {
    _tft.drawTriangle(x0, y0, x1, y1, x2, y2, color);
}

void TftDriver::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
                              int16_t x2, int16_t y2, uint16_t color) {
    _tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

// ==================== TEXT ====================

void TftDriver::setCursor(int16_t x, int16_t y) {
    _tft.setCursor(x, y);
}

void TftDriver::setTextColor(uint16_t color) {
    _tft.setTextColor(color);
}

void TftDriver::setTextColor(uint16_t color, uint16_t bg) {
    _tft.setTextColor(color, bg);
}

void TftDriver::setTextSize(uint8_t size) {
    _tft.setTextSize(size);
}

void TftDriver::setTextWrap(bool wrap) {
    _tft.setTextWrap(wrap);
}

void TftDriver::print(const String& text) {
    _tft.print(text);
}

void TftDriver::println(const String& text) {
    _tft.println(text);
}

void TftDriver::print(int value) {
    _tft.print(value);
}

void TftDriver::print(float value, int decimals) {
    _tft.print(value, decimals);
}

void TftDriver::printAt(int16_t x, int16_t y, const String& text, 
                         uint16_t color, uint8_t size) {
    _tft.setCursor(x, y);
    _tft.setTextColor(color);
    _tft.setTextSize(size);
    _tft.print(text);
}

// ==================== BITMAP/HINH ANH ====================

void TftDriver::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                            int16_t w, int16_t h, uint16_t color) {
    _tft.drawBitmap(x, y, bitmap, w, h, color);
}

void TftDriver::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                            int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    _tft.drawBitmap(x, y, bitmap, w, h, color, bg);
}

void TftDriver::drawRGBBitmap(int16_t x, int16_t y, const uint16_t* bitmap,
                               int16_t w, int16_t h) {
    _tft.drawRGBBitmap(x, y, bitmap, w, h);
}

// ==================== TIEN ICH MAU ====================

uint16_t TftDriver::color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

Adafruit_ST7735& TftDriver::getTft() {
    return _tft;
}

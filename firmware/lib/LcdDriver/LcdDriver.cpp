#include "LcdDriver.h"

LcdDriver::LcdDriver(uint8_t addr, uint8_t cols, uint8_t rows) 
    : _lcd(addr, cols, rows) {
}

void LcdDriver::init() {
    _lcd.init();
    _lcd.backlight();
    _lcd.clear();
}

void LcdDriver::clear() {
    _lcd.clear();
}

void LcdDriver::printAt(uint8_t col, uint8_t row, const String& text) {
    _lcd.setCursor(col, row);
    _lcd.print(text);
}

void LcdDriver::setBacklight(bool state) {
    if (state) {
        _lcd.backlight();
    } else {
        _lcd.noBacklight();
    }
}
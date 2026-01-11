#include "LcdHandler.h"
#include <cstdio>

LcdHandler::LcdHandler() 
    : _lcd(LCD_ADDR, LCD_COLS, LCD_ROWS) // Constants from bsp_board.h
{
}

void LcdHandler::begin() {
    _lcd.init();
    showStartup();
}

void LcdHandler::showStartup() {
    _lcd.clear();
    _lcd.printAt(0, 0, "  Welcome to  ");
}

void LcdHandler::showValueScreen(float ph, float temp) {
    _lcd.printAt(0, 0, "pH:   " + String(ph, 2) + " pH");
    char tempBuf[16];
    sprintf(tempBuf, "Temp: %.1f %cC", temp, (char)223);
    _lcd.printAt(0, 1, tempBuf);
}

void LcdHandler::showThresholdScreen(float upper, float lower) {
    _lcd.printAt(0, 0, "Upper: " + String(upper, 2));
    _lcd.printAt(0, 1, "Lower: " + String(lower, 2));
}

void LcdHandler::showError(const String& msg) {
    _lcd.clear();
    _lcd.printAt(0, 0, "Error:");
    _lcd.printAt(0, 1, msg);
}

void LcdHandler::showWiFiStatus(const String& status) {
    _lcd.clear();
    _lcd.printAt(0, 0, "WiFi Status:");
    _lcd.printAt(0, 1, status);
}
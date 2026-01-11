#include "LcdHandler.h"
#include <cstdio>

LcdHandler::LcdHandler(LcdDriver* driver) : _lcd(driver) {}

void LcdHandler::showStartup() {
    _lcd->clear();
    _lcd->printAt(0, 0, "  Welcome to  ");
}

void LcdHandler::showValueScreen(float ph, float temp) {
    _lcd->printAt(0, 0, "pH:   " + String(ph, 2) + " pH");
    char tempBuf[16];
    sprintf(tempBuf, "Temp: %.1f %cC", temp, (char)223);
    _lcd->printAt(0, 1, tempBuf);
}

void LcdHandler::showThresholdScreen(float upper, float lower) {
    _lcd->printAt(0, 0, "Upper: " + String(upper, 2));
    _lcd->printAt(0, 1, "Lower: " + String(lower, 2));
}
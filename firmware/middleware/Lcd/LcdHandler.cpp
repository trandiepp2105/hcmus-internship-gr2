#include "LcdHandler.h"
#include <cstdio>
#include <cstring>

// ConfigState values (matching PhContext.h)
#define CFG_THRESHOLD 0
#define CFG_SLOPE     1
#define CFG_INTERCEPT 2

// Helper function to pad string to 16 chars (LCD width)
static void padTo16(char* str) {
    size_t len = strlen(str);
    if (len < 16) {
        memset(str + len, ' ', 16 - len);
        str[16] = '\0';
    }
}

LcdHandler::LcdHandler() 
    : _lcd(LCD_ADDR, LCD_COLS, LCD_ROWS) // Constants from bsp_board.h
{
}

void LcdHandler::begin() {
    Serial.println("[LcdHandler] Initializing LCD...");
    _lcd.init();
    _lcd.setBacklight(255); // Ensure backlight specifically here too
    showStartup();
    Serial.println("[LcdHandler] LCD Init Done.");
}

void LcdHandler::showStartup() {
    _lcd.clear();
    _lcd.printAt(0, 0, "pH Controller   ");
    _lcd.printAt(0, 1, "Starting...     ");
}

void LcdHandler::showAutoManualScreen(float ph, float temp, bool out1, bool out2, bool out3, bool out4, bool isAuto) {
    // Row 0: "PH:XX.XX T:XX.X" (16 chars max)
    char row0[17];
    snprintf(row0, sizeof(row0), "PH:%.2f T:%.1f", ph, temp);
    padTo16(row0);
    _lcd.printAt(0, 0, row0);
    
    // Row 1: "M:AT OUT:XXXX" or "M:MN OUT:XXXX"
    char row1[17];
    const char* modeStr = isAuto ? "AT" : "MN";
    snprintf(row1, sizeof(row1), "M:%s OUT:%d%d%d%d", 
             modeStr, out1 ? 1 : 0, out2 ? 1 : 0, out3 ? 1 : 0, out4 ? 1 : 0);
    padTo16(row1);
    _lcd.printAt(0, 1, row1);
}

void LcdHandler::showConfigScreen(int cfgState, float val1, float val2) {
    char row0[17];
    char row1[17];
    
    switch (cfgState) {
        case CFG_THRESHOLD:
            // Row 0: "Up:X.X Lw:X.X"
            snprintf(row0, sizeof(row0), "Up:%.1f Lw:%.1f", val1, val2);
            snprintf(row1, sizeof(row1), "M:CF THRESHOLD");
            break;
            
        case CFG_SLOPE:
            // Row 0: Show slope value
            snprintf(row0, sizeof(row0), "Slope: %.4f", val1);
            snprintf(row1, sizeof(row1), "M:CF CALIB");
            break;
            
        case CFG_INTERCEPT:
            // Row 0: Show intercept value
            snprintf(row0, sizeof(row0), "Intcpt: %.3f", val1);
            snprintf(row1, sizeof(row1), "M:CF CALIB");
            break;
            
        default:
            snprintf(row0, sizeof(row0), "Unknown Config");
            snprintf(row1, sizeof(row1), "M:CF ERROR");
            break;
    }
    
    padTo16(row0);
    padTo16(row1);
    _lcd.printAt(0, 0, row0);
    _lcd.printAt(0, 1, row1);
}

void LcdHandler::showInfoScreen(float upper, float lower, float slope, float intercept) {
    // Cycle through info pages using millis()
    // Page 0: Thresholds | Page 1: Calibration
    static unsigned long lastPageChange = 0;
    static int page = 0;
    const unsigned long PAGE_INTERVAL = 3000; // 3 seconds per page
    
    if (millis() - lastPageChange >= PAGE_INTERVAL) {
        page = (page + 1) % 2; // Toggle between 2 pages
        lastPageChange = millis();
    }
    
    char row0[17];
    char row1[17];
    
    if (page == 0) {
        // Page 0: Thresholds
        snprintf(row0, sizeof(row0), "Up:%.1f Lw:%.1f", upper, lower);
        snprintf(row1, sizeof(row1), "M:IF THRESHOLD");
    } else {
        // Page 1: Calibration
        snprintf(row0, sizeof(row0), "S:%.2f I:%.2f", slope, intercept);
        snprintf(row1, sizeof(row1), "M:IF CALIB");
    }
    
    padTo16(row0);
    padTo16(row1);
    _lcd.printAt(0, 0, row0);
    _lcd.printAt(0, 1, row1);
}

void LcdHandler::showError(const String& msg) {
    _lcd.clear();
    _lcd.printAt(0, 0, "ERROR:          ");
    char row1[17];
    snprintf(row1, sizeof(row1), "%s", msg.c_str());
    padTo16(row1);
    _lcd.printAt(0, 1, row1);
}

void LcdHandler::showWiFiStatus(const String& status) {
    _lcd.clear();
    _lcd.printAt(0, 0, "WiFi:           ");
    char row1[17];
    snprintf(row1, sizeof(row1), "%s", status.c_str());
    padTo16(row1);
    _lcd.printAt(0, 1, row1);
}
#include "TftHandler.h"
#include <cstdio>

// ConfigState values
#define CFG_THRESHOLD 0
#define CFG_SLOPE     1
#define CFG_INTERCEPT 2

// Font size 1: 6x8 pixels per character
// Screen 160x128: max 26 chars/line, 16 lines

TftHandler::TftHandler()
    : _tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST) {
    // Using hardware SPI (faster) - MOSI=23, SCK=18 auto-configured
}

void TftHandler::begin() {
    Serial.println("[TftHandler] Initializing TFT...");
    _tft.init(INITR_BLACKTAB);
    _tft.setRotation(1);  // Landscape: 160x128
    _tft.setTextWrap(false);  // Disable auto text wrap
    _tft.fillScreen(COLOR_BG_LIGHT);
    showStartup();
    Serial.println("[TftHandler] TFT Init Done.");
}

void TftHandler::resetOnModeChange() {
    // Force full redraw on next screen call
    _firstDraw = true;
    _lastPh = -1.0f;
    _lastTemp = -1.0f;
    _lastR1 = _lastR2 = _lastR3 = _lastR4 = false;
    _lastIsAuto = true;
    
    // Reset CONFIG and INFO state
    _lastCfgState = -1;
    _lastConfigVal1 = -999.0f;
    _lastConfigVal2 = -999.0f;
    _infoDrawn = false;
}

// ==================== HELPER METHODS ====================

void TftHandler::drawHeader(const String& title, const String& mode) {
    _tft.fillRect(0, 0, TFT_WIDTH, HEADER_HEIGHT, COLOR_HEADER_BG);
    
    // Only show mode, centered, size 1, bold black text
    if (mode.length() > 0) {
        _tft.setTextColor(COLOR_TEXT_PRIMARY);
        _tft.setTextSize(1);
        // Center the mode text
        int16_t textWidth = mode.length() * 6; // 6 pixels per char
        int16_t x = (TFT_WIDTH - textWidth) / 2;
        _tft.setCursor(x, 5);
        _tft.print(mode);
    }
}

void TftHandler::drawFooter(const String& hint) {
    int16_t footerY = TFT_HEIGHT - FOOTER_HEIGHT;
    _tft.fillRect(0, footerY, TFT_WIDTH, FOOTER_HEIGHT, COLOR_FOOTER_BG);
    _tft.setTextColor(COLOR_TEXT_SECONDARY);
    _tft.setTextSize(1);
    _tft.setCursor(2, footerY + 8);
    // Truncate hint to fit (max 26 chars)
    if (hint.length() > 26) {
        _tft.print(hint.substring(0, 26));
    } else {
        _tft.print(hint);
    }
}

void TftHandler::drawRelayBar(bool r1, bool r2, bool r3, bool r4) {
    int16_t barY = TFT_HEIGHT - FOOTER_HEIGHT;
    _tft.fillRect(0, barY, TFT_WIDTH, FOOTER_HEIGHT, COLOR_FOOTER_BG);
    
    // Compact format: "R1+ R2- R3- R4-" (fits in 160px)
    // Each relay uses about 35px
    const int spacing = 40;
    bool states[] = {r1, r2, r3, r4};
    
    for (int i = 0; i < 4; i++) {
        int16_t x = i * spacing + 2;
        // ON = Green, OFF = Red
        uint16_t color = states[i] ? COLOR_RELAY_ON : COLOR_DISCONNECTED;
        _tft.setTextColor(color);
        _tft.setCursor(x, barY + 8);
        _tft.print("R");
        _tft.print(i + 1);
        _tft.print(states[i] ? "+" : "-");
    }
}

uint16_t TftHandler::getPhColor(float ph, float upper, float lower) {
    if (ph > upper) return COLOR_PH_HIGH;
    if (ph < lower) return COLOR_PH_LOW;
    return COLOR_PH_OK;
}

// ==================== MAIN SCREENS ====================

void TftHandler::showAutoManualScreen(float ph, float temp,
                                       bool r1, bool r2, bool r3, bool r4,
                                       bool isAuto, float upper, float lower) {
    // First draw: clear everything and draw static elements
    if (_firstDraw) {
        _tft.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, COLOR_BG_LIGHT);
        drawHeader("", isAuto ? "AUTOMATIC" : "MANUAL");
        _lastIsAuto = isAuto;
        
        // Draw thresholds (static on AUTO/MANUAL screen)
        int16_t rightX = 70;
        _tft.setTextColor(COLOR_PH_HIGH);
        _tft.setTextSize(1);
        _tft.setCursor(rightX, 40);
        char upperStr[10];
        snprintf(upperStr, sizeof(upperStr), "Hi:%.1f", upper);
        _tft.print(upperStr);
        
        _tft.setTextColor(COLOR_PH_LOW);
        _tft.setCursor(rightX, 55);
        char lowerStr[10];
        snprintf(lowerStr, sizeof(lowerStr), "Lo:%.1f", lower);
        _tft.print(lowerStr);
        
        // Draw footer immediately
        drawRelayBar(r1, r2, r3, r4);
        _lastR1 = r1; _lastR2 = r2; _lastR3 = r3; _lastR4 = r4;
        
        _firstDraw = false;
    }
    
    // Update header only if mode changed
    if (_lastIsAuto != isAuto) {
        drawHeader("", isAuto ? "AUTOMATIC" : "MANUAL");
        _lastIsAuto = isAuto;
    }
    
    uint16_t phColor = getPhColor(ph, upper, lower);
    
    // Update pH value only if changed
    if (abs(ph - _lastPh) > 0.01 || _lastPh < 0) {
        // Erase old pH value and circles
        if (_lastPh >= 0) {
            char oldPhStr[6];
            snprintf(oldPhStr, sizeof(oldPhStr), "%.1f", _lastPh);
            _tft.setTextColor(COLOR_BG_LIGHT);
            _tft.setTextSize(2);
            _tft.setCursor(18, 51);
            _tft.print(oldPhStr);
            
            _tft.drawCircle(35, 58, 22, COLOR_BG_LIGHT);
            _tft.drawCircle(35, 58, 21, COLOR_BG_LIGHT);
        }
        
        // Draw new pH circles
        _tft.drawCircle(35, 58, 22, phColor);
        _tft.drawCircle(35, 58, 21, phColor);
        
        // Draw new pH value
        char phStr[6];
        snprintf(phStr, sizeof(phStr), "%.1f", ph);
        _tft.setTextColor(phColor);
        _tft.setTextSize(2);
        _tft.setCursor(18, 51);
        _tft.print(phStr);
        
        // pH label
        _tft.setTextSize(1);
        _tft.setTextColor(phColor);
        _tft.setCursor(30, 85);
        _tft.print("pH");
        
        _lastPh = ph;
    }
    
    int16_t rightX = 70;
    
    // Update temperature only if changed
    if (abs(temp - _lastTemp) > 0.1 || _lastTemp < 0) {
        // Erase old temp
        if (_lastTemp >= 0) {
            _tft.fillRect(rightX, 24, 60, 8, COLOR_BG_LIGHT);
        }
        
        // Draw new temp
        _tft.setTextColor(COLOR_TEXT_PRIMARY);
        _tft.setTextSize(1);
        _tft.setCursor(rightX, 24);
        char tempStr[12];
        snprintf(tempStr, sizeof(tempStr), "T:%.1fC", temp);
        _tft.print(tempStr);
        
        _lastTemp = temp;
    }
    
    // Update status text when pH zone changes
    static int lastZone = -1;
    int currentZone = (ph > upper) ? 2 : (ph < lower) ? 0 : 1;
    
    if (currentZone != lastZone) {
        _tft.fillRect(rightX, 75, 60, 8, COLOR_BG_LIGHT);
        
        _tft.setTextColor(phColor);
        _tft.setTextSize(1);
        _tft.setCursor(rightX, 75);
        
        if (ph > upper) {
            _tft.print("HIGH!");
        } else if (ph < lower) {
            _tft.print("LOW!");
        } else {
            _tft.print("OK");
        }
        
        lastZone = currentZone;
    }
    
    // Update relay bar only if changed
    if (r1 != _lastR1 || r2 != _lastR2 || r3 != _lastR3 || r4 != _lastR4) {
        drawRelayBar(r1, r2, r3, r4);
        _lastR1 = r1; _lastR2 = r2; _lastR3 = r3; _lastR4 = r4;
    }
}

void TftHandler::showConfigScreen(int cfgState, float val1, float val2) {
    // Only draw everything on mode change or first call
    if (_lastCfgState != cfgState) {
        _tft.fillScreen(COLOR_BG_LIGHT);
        drawHeader("", "CONFIGURATION");
        _lastCfgState = cfgState;
        
        // Draw static labels
        _tft.setTextColor(COLOR_TEXT_PRIMARY);
        _tft.setTextSize(1);
        
        if (cfgState == CFG_THRESHOLD) {
            _tft.setCursor(5, 30);
            _tft.print("Upper:");
            _tft.setCursor(5, 55);
            _tft.print("Lower:");
        } else if (cfgState == CFG_SLOPE) {
            _tft.setCursor(10, 40);
            _tft.print("Slope:");
        } else {
            _tft.setCursor(10, 40);
            _tft.print("Intercept:");
        }
        
        // Draw footer
        drawFooter("Rotate pot    [A]Save");
    }
    
    // Update only the values (partial update)
    
    if (cfgState == CFG_THRESHOLD) {
        // Update upper value if changed
        if (abs(val1 - _lastConfigVal1) > 0.01) {
            // Clear value area
            _tft.fillRect(50, 26, 48, 16, COLOR_BG_LIGHT);
            _tft.setTextColor(COLOR_PH_HIGH);
            _tft.setTextSize(2);
            _tft.setCursor(50, 26);
            char v1[8];
            snprintf(v1, sizeof(v1), "%.2f", val1);
            _tft.print(v1);
            
            // Update progress bar
            int bar1 = (int)((val1 / 14.0) * 60);
            _tft.fillRect(100, 28, 55, 8, COLOR_FOOTER_BG);
            _tft.fillRect(100, 28, bar1, 8, COLOR_PH_HIGH);
            
            _lastConfigVal1 = val1;
        }
        
        // Update lower value if changed
        if (abs(val2 - _lastConfigVal2) > 0.01) {
            // Clear value area
            _tft.fillRect(50, 51, 48, 16, COLOR_BG_LIGHT);
            _tft.setTextColor(COLOR_PH_LOW);
            _tft.setTextSize(2);
            _tft.setCursor(50, 51);
            char v2[8];
            snprintf(v2, sizeof(v2), "%.2f", val2);
            _tft.print(v2);
            
            // Update progress bar
            int bar2 = (int)((val2 / 14.0) * 60);
            _tft.fillRect(100, 53, 55, 8, COLOR_FOOTER_BG);
            _tft.fillRect(100, 53, bar2, 8, COLOR_PH_LOW);
            
            _lastConfigVal2 = val2;
        }
        
    } else {
        // For slope/intercept, only update value
        if (abs(val1 - _lastConfigVal1) > 0.001) {
            _tft.fillRect(20, 55, 120, 16, COLOR_BG_LIGHT);
            _tft.setTextColor(COLOR_TEXT_PRIMARY);
            _tft.setTextSize(2);
            _tft.setCursor(20, 55);
            char valStr[12];
            if (cfgState == CFG_SLOPE) {
                snprintf(valStr, sizeof(valStr), "%.4f", val1);
            } else {
                snprintf(valStr, sizeof(valStr), "%.3f", val1);
            }
            _tft.print(valStr);
            _lastConfigVal1 = val1;
        }
    }
}

void TftHandler::showInfoScreen(bool wifiConnected, bool mqttConnected,
                                 const String& ip, const String& deviceName,
                                 float upper, float lower, float slope, float intercept,
                                 const String& nextModeName) {
    // Only draw once - INFO screen is static
    if (!_infoDrawn) {
        _tft.fillScreen(COLOR_BG_LIGHT);
        
        // Header: Show INFORMATION mode
        drawHeader("", "INFORMATION");
        
        _tft.setTextSize(1);
        int16_t y = 22;
        
        // WiFi status (compact)
        _tft.setTextColor(wifiConnected ? COLOR_CONNECTED : COLOR_DISCONNECTED);
        _tft.setCursor(2, y);
        _tft.print(wifiConnected ? "WiFi:ON" : "WiFi:--");
        
        // IP (truncated)
        _tft.setTextColor(COLOR_TEXT_SECONDARY);
        _tft.setCursor(55, y);
        String shortIp = ip.length() > 15 ? ip.substring(0, 15) : ip;
        _tft.print(shortIp);
        
        y += 12;
        
        // MQTT status
        _tft.setTextColor(mqttConnected ? COLOR_CONNECTED : COLOR_DISCONNECTED);
        _tft.setCursor(2, y);
        _tft.print(mqttConnected ? "MQTT:ON" : "MQTT:--");
        
        y += 14;
        
        // Separator
        _tft.drawFastHLine(2, y, TFT_WIDTH - 4, COLOR_FOOTER_BG);
        y += 4;
        
        // Thresholds
        _tft.setTextColor(COLOR_TEXT_PRIMARY);
        _tft.setCursor(2, y);
        char thStr[24];
        snprintf(thStr, sizeof(thStr), "Hi:%.1f Lo:%.1f", upper, lower);
        _tft.print(thStr);
        
        y += 12;
        
        // Calibration
        _tft.setCursor(2, y);
        char calStr[24];
        snprintf(calStr, sizeof(calStr), "Sl:%.2f In:%.2f", slope, intercept);
        _tft.print(calStr);
        
        // Footer
        String hint = "[A]->" + nextModeName;
        drawFooter(hint);
        
        _infoDrawn = true;
    }
    // INFO screen is static - don't redraw
}

// ==================== UTILITY SCREENS ====================

void TftHandler::showStartup() {
    _tft.fillScreen(COLOR_BG_LIGHT);
    _tft.setTextColor(COLOR_TEXT_PRIMARY);
    _tft.setTextSize(2);
    _tft.setCursor(30, 50);
    _tft.print("pH CTRL");
    _tft.setTextSize(1);
    _tft.setTextColor(COLOR_TEXT_SECONDARY);
    _tft.setCursor(45, 75);
    _tft.print("Starting...");
}

void TftHandler::showError(const String& msg) {
    _tft.fillScreen(COLOR_PH_HIGH);
    _tft.setTextColor(COLOR_TEXT_PRIMARY);
    _tft.setTextSize(2);
    _tft.setCursor(45, 45);
    _tft.print("ERROR");
    _tft.setTextSize(1);
    _tft.setCursor(5, 75);
    _tft.print(msg.substring(0, 26)); // max 26 chars
}

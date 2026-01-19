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

void TftHandler::drawRelayToggleBar(bool r1, bool r2, bool r3, bool r4) {
    int16_t barY = TFT_HEIGHT - FOOTER_HEIGHT;
    _tft.fillRect(0, barY, TFT_WIDTH, FOOTER_HEIGHT, COLOR_FOOTER_BG);
    
    // Toggle switch style: R1 [ON/OFF] R2 [ON/OFF] R3 [ON/OFF] R4 [ON/OFF]
    bool states[] = {r1, r2, r3, r4};
    const int16_t toggleW = 35;  // Width per toggle widget
    const int16_t startX = 5;
    
    for (int i = 0; i < 4; i++) {
        int16_t x = startX + i * toggleW + i * 5;
        
        // Relay label
        _tft.setTextColor(COLOR_TEXT_PRIMARY);
        _tft.setTextSize(1);
        _tft.setCursor(x + 5, barY + 3);
        _tft.print("R");
        _tft.print(i + 1);
        
        // Toggle switch box
        int16_t boxX = x;
        int16_t boxY = barY + 12;
        int16_t boxW = 30;
        int16_t boxH = 10;
        
        // Box background
        uint16_t bgColor = states[i] ? COLOR_RELAY_ON : 0xC618; // Green or gray
        _tft.fillRect(boxX, boxY, boxW, boxH, bgColor);
        _tft.drawRect(boxX, boxY, boxW, boxH, COLOR_TEXT_PRIMARY);
        
        // Toggle indicator (slider position)
        int16_t sliderX = states[i] ? (boxX + boxW - 12) : boxX + 2;
        _tft.fillRect(sliderX, boxY + 2, 10, boxH - 4, 0xFFFF);
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
    // Layout constants for new design
    const int16_t LEFT_WIDTH = 70;      // pH gauge area
    const int16_t RIGHT_X = 72;         // Right panel start
    const int16_t BAR_WIDTH = 70;       // Progress bar width
    const int16_t BAR_HEIGHT = 10;      // Progress bar height
    
    // Static vars for Hi/Lo tracking - MUST be declared before _firstDraw check
    static float lastUpper = -999;
    static float lastLower = -999;
    
    // First draw: clear and draw all static elements
    if (_firstDraw) {
        _tft.fillScreen(COLOR_BG_LIGHT);
        
        // Header
        _tft.fillRect(0, 0, TFT_WIDTH, HEADER_HEIGHT, 0x31A6); // Dark gray header
        _tft.setTextColor(0xFFFF); // White text
        _tft.setTextSize(1);
        int16_t modeX = (TFT_WIDTH - (isAuto ? 4 : 6) * 6) / 2;
        _tft.setCursor(modeX, 5);
        _tft.print(isAuto ? "AUTO" : "MANUAL");
        _lastIsAuto = isAuto;
        
        // pH label below gauge
        _tft.setTextColor(COLOR_TEXT_SECONDARY);
        _tft.setTextSize(1);
        _tft.setCursor(28, 88);
        _tft.print("pH");
        
        // Footer with relay toggles
        drawRelayToggleBar(r1, r2, r3, r4);
        _lastR1 = r1; _lastR2 = r2; _lastR3 = r3; _lastR4 = r4;
        
        // CRITICAL: Force Hi/Lo bars to draw on first draw
        lastUpper = -999;
        lastLower = -999;
        
        _firstDraw = false;
    }
    
    // Update header if mode changed
    if (_lastIsAuto != isAuto) {
        _tft.fillRect(0, 0, TFT_WIDTH, HEADER_HEIGHT, 0x31A6);
        _tft.setTextColor(0xFFFF);
        _tft.setTextSize(1);
        int16_t modeX = (TFT_WIDTH - (isAuto ? 4 : 6) * 6) / 2;
        _tft.setCursor(modeX, 5);
        _tft.print(isAuto ? "AUTO" : "MANUAL");
        _lastIsAuto = isAuto;
        // Force Hi/Lo bars to redraw on mode change
        lastUpper = -999;
        lastLower = -999;
    }
    
    uint16_t phColor = getPhColor(ph, upper, lower);
    
    // === LEFT SIDE: pH Gauge ===
    if (abs(ph - _lastPh) > 0.01 || _lastPh < 0) {
        // Clear pH gauge area
        _tft.fillRect(5, 25, 60, 60, COLOR_BG_LIGHT);
        
        // Draw pH gauge arc - using thick lines for solid appearance
        int16_t cx = 35, cy = 55;
        int16_t innerR = 22;
        int16_t outerR = 28;
        
        // Draw solid filled arc sections using multiple circles
        // Blue section (Low: 180-220 degrees = acidic zone)
        for (int angle = 180; angle < 220; angle++) {
            float rad = angle * 3.14159 / 180.0;
            float cosA = cos(rad);
            float sinA = sin(rad);
            // Draw line from inner to outer radius
            for (int r = innerR; r <= outerR; r++) {
                int16_t x = cx + cosA * r;
                int16_t y = cy + sinA * r;
                _tft.drawPixel(x, y, COLOR_PH_LOW);
            }
        }
        
        // Green section (OK: 220-320 degrees = neutral zone)
        for (int angle = 220; angle < 320; angle++) {
            float rad = angle * 3.14159 / 180.0;
            float cosA = cos(rad);
            float sinA = sin(rad);
            for (int r = innerR; r <= outerR; r++) {
                int16_t x = cx + cosA * r;
                int16_t y = cy + sinA * r;
                _tft.drawPixel(x, y, COLOR_PH_OK);
            }
        }
        
        // Red section (High: 320-360 degrees = alkaline zone)
        for (int angle = 320; angle <= 360; angle++) {
            float rad = angle * 3.14159 / 180.0;
            float cosA = cos(rad);
            float sinA = sin(rad);
            for (int r = innerR; r <= outerR; r++) {
                int16_t x = cx + cosA * r;
                int16_t y = cy + sinA * r;
                _tft.drawPixel(x, y, COLOR_PH_HIGH);
            }
        }
        
        // Draw pH value in center
        char phStr[6];
        snprintf(phStr, sizeof(phStr), "%.1f", ph);
        _tft.setTextColor(phColor);
        _tft.setTextSize(2);
        int16_t textW = strlen(phStr) * 12;
        _tft.setCursor(cx - textW/2, cy - 5);
        _tft.print(phStr);
        
        _lastPh = ph;
    }
    
    // === RIGHT SIDE: Hi/Lo Bars + Temp ===
    
    // Hi bar (30-50 Y)
    if (upper != lastUpper) {
        _tft.fillRect(RIGHT_X, 25, 88, 22, COLOR_BG_LIGHT);
        _tft.setTextColor(COLOR_PH_HIGH);
        _tft.setTextSize(1);
        _tft.setCursor(RIGHT_X, 27);
        char hiLabel[12];
        snprintf(hiLabel, sizeof(hiLabel), "Hi: %.1f", upper);
        _tft.print(hiLabel);
        
        // Progress bar below label
        _tft.fillRect(RIGHT_X, 38, BAR_WIDTH, BAR_HEIGHT, 0xC618);
        int16_t fillW = (upper / 14.0) * BAR_WIDTH;
        _tft.fillRect(RIGHT_X, 38, fillW, BAR_HEIGHT, COLOR_PH_HIGH);
        
        lastUpper = upper;
    }
    
    // Lo bar (52-72 Y)
    if (lower != lastLower) {
        _tft.fillRect(RIGHT_X, 50, 88, 22, COLOR_BG_LIGHT);
        _tft.setTextColor(COLOR_PH_LOW);
        _tft.setTextSize(1);
        _tft.setCursor(RIGHT_X, 52);
        char loLabel[12];
        snprintf(loLabel, sizeof(loLabel), "Lo: %.1f", lower);
        _tft.print(loLabel);
        
        // Progress bar below label
        _tft.fillRect(RIGHT_X, 63, BAR_WIDTH, BAR_HEIGHT, 0xC618);
        int16_t fillW = (lower / 14.0) * BAR_WIDTH;
        _tft.fillRect(RIGHT_X, 63, fillW, BAR_HEIGHT, COLOR_PH_LOW);
        
        lastLower = lower;
    }
    
    // Temperature (80-95 Y)
    if (abs(temp - _lastTemp) > 0.1 || _lastTemp < 0) {
        _tft.fillRect(RIGHT_X, 80, 80, 15, COLOR_BG_LIGHT);
        _tft.setTextColor(COLOR_TEXT_PRIMARY);
        _tft.setTextSize(1);
        _tft.setCursor(RIGHT_X, 82);
        char tempStr[15];
        snprintf(tempStr, sizeof(tempStr), "Temp: %.1fC", temp);
        _tft.print(tempStr);
        _lastTemp = temp;
    }
    
    // === FOOTER: Relay Toggle Bar ===
    if (r1 != _lastR1 || r2 != _lastR2 || r3 != _lastR3 || r4 != _lastR4) {
        drawRelayToggleBar(r1, r2, r3, r4);
        _lastR1 = r1; _lastR2 = r2; _lastR3 = r3; _lastR4 = r4;
    }
}

void TftHandler::showConfigScreen(int cfgState, float val1, float val2) {
    // New layout: Header | UPPER (left) | LOWER (right) | pH Scale | Footer
    
    // Check if need full redraw
    if (_lastCfgState != cfgState) {
        _tft.fillScreen(COLOR_BG_LIGHT);
        _lastCfgState = cfgState;
        _lastConfigVal1 = -999;
        _lastConfigVal2 = -999;
        
        // Dark header
        _tft.fillRect(0, 0, TFT_WIDTH, HEADER_HEIGHT, 0x31A6);
        _tft.setTextColor(0xFFFF);
        _tft.setTextSize(1);
        
        if (cfgState == CFG_THRESHOLD) {
            _tft.setCursor(25, 5);
            _tft.print("THRESHOLD CONFIG");
            
            // Divider line between UPPER and LOWER
            _tft.drawFastVLine(80, 22, 60, 0xC618);
            
        } else {
            _tft.setCursor(30, 5);
            _tft.print("CALIB CONFIG");
            
            // Divider line
            _tft.drawFastVLine(80, 22, 60, 0xC618);
        }
        
        // Footer
        _tft.fillRect(0, TFT_HEIGHT - FOOTER_HEIGHT, TFT_WIDTH, FOOTER_HEIGHT, COLOR_FOOTER_BG);
        _tft.setTextColor(COLOR_TEXT_SECONDARY);
        _tft.setTextSize(1);
        _tft.setCursor(10, TFT_HEIGHT - 16);
        _tft.print("Rotate pot | [THR] Save");
    }
    
    // === Update values (partial update) ===
    if (cfgState == CFG_THRESHOLD) {
        // LEFT: UPPER value
        if (abs(val1 - _lastConfigVal1) > 0.01) {
            _tft.fillRect(5, 25, 70, 55, COLOR_BG_LIGHT);
            
            // UPPER label
            _tft.setTextColor(COLOR_TEXT_SECONDARY);
            _tft.setTextSize(1);
            _tft.setCursor(22, 28);
            _tft.print("UPPER");
            
            // Large value
            _tft.setTextColor(COLOR_PH_HIGH);  // Red
            _tft.setTextSize(3);
            char v1[6];
            snprintf(v1, sizeof(v1), "%.1f", val1);
            int16_t w = strlen(v1) * 18;
            _tft.setCursor(40 - w/2, 45);
            _tft.print(v1);
            
            _lastConfigVal1 = val1;
        }
        
        // RIGHT: LOWER value
        if (abs(val2 - _lastConfigVal2) > 0.01) {
            _tft.fillRect(85, 25, 70, 55, COLOR_BG_LIGHT);
            
            // LOWER label
            _tft.setTextColor(COLOR_TEXT_SECONDARY);
            _tft.setTextSize(1);
            _tft.setCursor(102, 28);
            _tft.print("LOWER");
            
            // Large value
            _tft.setTextColor(COLOR_PH_LOW);  // Blue
            _tft.setTextSize(3);
            char v2[6];
            snprintf(v2, sizeof(v2), "%.1f", val2);
            int16_t w = strlen(v2) * 18;
            _tft.setCursor(120 - w/2, 45);
            _tft.print(v2);
            
            _lastConfigVal2 = val2;
        }
        
    } else {
        // CALIB mode: SLOPE (left) / INTERCEPT (right)
        if (abs(val1 - _lastConfigVal1) > 0.001) {
            _tft.fillRect(5, 25, 70, 55, COLOR_BG_LIGHT);
            
            _tft.setTextColor(COLOR_TEXT_SECONDARY);
            _tft.setTextSize(1);
            _tft.setCursor(18, 28);
            _tft.print(cfgState == CFG_SLOPE ? "SLOPE" : "INTERCEPT");
            
            _tft.setTextColor(COLOR_PH_OK);  // Green
            _tft.setTextSize(2);
            char valStr[10];
            snprintf(valStr, sizeof(valStr), "%.3f", val1);
            int16_t w = strlen(valStr) * 12;
            _tft.setCursor(40 - w/2, 50);
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
        String hint = "[MODE]->" + nextModeName;
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

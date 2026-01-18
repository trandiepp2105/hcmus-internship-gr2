#ifndef TFT_HANDLER_H
#define TFT_HANDLER_H

#include "../../lib/TftDriver/TftDriver.h"
#include "../bsp_board.h"

// Screen dimensions (landscape)
#define TFT_WIDTH  160
#define TFT_HEIGHT 128

// Layout constants
#define HEADER_HEIGHT    18
#define FOOTER_HEIGHT    24
#define CONTENT_TOP      HEADER_HEIGHT
#define CONTENT_HEIGHT   (TFT_HEIGHT - HEADER_HEIGHT - FOOTER_HEIGHT)

// Color palette - LIGHT THEME
#define COLOR_BG_LIGHT      0xFFFF  // White background
#define COLOR_HEADER_BG     0xAEDC  // Light cyan/sky blue header
#define COLOR_FOOTER_BG     0xDEFB  // Light gray footer
#define COLOR_TEXT_PRIMARY  0x0000  // Black text
#define COLOR_TEXT_SECONDARY 0x4208 // Dark gray text
#define COLOR_PH_OK         0x03E0  // Dark Green (was 0x07E0)
#define COLOR_PH_HIGH       0xF800  // Red
#define COLOR_PH_LOW        0x001F  // Blue
#define COLOR_RELAY_ON      0x03E0  // Dark Green (was 0x07E0)
#define COLOR_RELAY_OFF     0xF800  // Red (was gray)
#define COLOR_CONNECTED     0x07E0  // Green
#define COLOR_DISCONNECTED  0xF800  // Red

/**
 * @class TftHandler
 * @brief Middleware quan ly man hinh TFT ST7735 (160x128 landscape)
 */
class TftHandler {
public:
    TftHandler();

    void begin();
    
    /**
     * @brief Reset display state when mode changes (forces full redraw)
     * Call this when switching between AUTO/MANUAL/CONFIG/INFO
     */
    void resetOnModeChange();

    // === Main Screens ===
    
    /**
     * @brief Hien thi man hinh AUTO/MANUAL
     * @param ph Gia tri pH hien tai
     * @param temp Nhiet do hien tai
     * @param r1-r4 Trang thai 4 relay (true = ON)
     * @param isAuto true neu dang o che do AUTO
     * @param upper Nguong tren
     * @param lower Nguong duoi
     */
    void showAutoManualScreen(float ph, float temp, 
                               bool r1, bool r2, bool r3, bool r4,
                               bool isAuto, float upper, float lower);
    
    /**
     * @brief Hien thi man hinh CONFIG
     * @param cfgState 0=THRESHOLD, 1=SLOPE, 2=INTERCEPT
     * @param val1 Gia tri 1 (upper/slope)
     * @param val2 Gia tri 2 (lower/intercept)
     */
    void showConfigScreen(int cfgState, float val1, float val2);
    
    /**
     * @brief Hien thi man hinh INFO
     * @param wifiConnected Trang thai WiFi
     * @param mqttConnected Trang thai MQTT
     * @param ip Dia chi IP
     * @param deviceName Ten thiet bi
     * @param upper Nguong tren
     * @param lower Nguong duoi
     * @param slope He so slope
     * @param intercept He so intercept
     * @param nextModeName Ten mode tiep theo khi nhan A
     */
    void showInfoScreen(bool wifiConnected, bool mqttConnected,
                        const String& ip, const String& deviceName,
                        float upper, float lower, float slope, float intercept,
                        const String& nextModeName);
    
    // === Utility Screens ===
    void showStartup();
    void showError(const String& msg);

private:
    TftDriver _tft;
    
    // Track previous values for partial updates
    float _lastPh = -1.0f;
    float _lastTemp = -1.0f;
    bool _lastR1 = false, _lastR2 = false, _lastR3 = false, _lastR4 = false;
    bool _lastIsAuto = true;
    String _lastMode = "";
    bool _firstDraw = true;
    
    // CONFIG screen state
    int _lastCfgState = -1;
    float _lastConfigVal1 = -999.0f;
    float _lastConfigVal2 = -999.0f;
    
    // INFO screen state
    bool _infoDrawn = false;
    
    // Helper methods
    void drawHeader(const String& title, const String& mode = "");
    void drawFooter(const String& hint);
    void drawRelayBar(bool r1, bool r2, bool r3, bool r4);
    uint16_t getPhColor(float ph, float upper, float lower);
};

#endif

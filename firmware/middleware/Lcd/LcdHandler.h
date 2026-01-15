#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include "LcdDriver.h"
#include "../bsp_board.h" // Access Pin Defs

/**
 * @class LcdHandler
 * @brief Lop BSP quan ly LCD
 * Wraps LcdDriver and provides High-Level display methods.
 * 
 * LCD Layout (16x2):
 * Row 0: Content based on mode
 * Row 1: Mode indicator + additional info
 */
class LcdHandler {
public:
    LcdHandler(); // Constructor no longer needs external driver

    void begin(); // Init Driver

    // === New API for unified layout ===
    
    /**
     * @brief Show AUTO/MANUAL mode screen
     * Row 0: "PH:XX.XX T:XX.X"
     * Row 1: "M:AT OUT:XXXX" or "M:MN OUT:XXXX"
     */
    void showAutoManualScreen(float ph, float temp, bool out1, bool out2, bool out3, bool out4, bool isAuto);
    
    /**
     * @brief Show CONFIG mode screen
     * Row 0: Config parameter name
     * Row 1: "M:CF <value>"
     * @param cfgState 0=THRESHOLD, 1=SLOPE, 2=INTERCEPT
     */
    void showConfigScreen(int cfgState, float val1, float val2);
    
    /**
     * @brief Show INFO mode screen - cycles through config values
     * Row 0: Config info line 1
     * Row 1: "M:IF <info>"
     */
    void showInfoScreen(float upper, float lower, float slope, float intercept);
    
    // === Legacy API (kept for compatibility) ===
    void showStartup();
    void showError(const String& msg);
    void showWiFiStatus(const String& status);

private:
    LcdDriver _lcd; // Composition: Owns the driver
};

#endif
#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include "LcdDriver.h"
#include "../bsp_board.h" // Access Pin Defs

/**
 * @class LcdHandler
 * @brief Lop BSP quan ly LCD
 * Wraps LcdDriver and provides High-Level display methods.
 */
class LcdHandler {
public:
    LcdHandler(); // Constructor no longer needs external driver

    void begin(); // Init Driver

    // ... Existing API ...
    void showStartup();
    void showValueScreen(float ph, float temp);
    void showThresholdScreen(float upper, float lower);
    void showError(const String& msg);
    void showWiFiStatus(const String& status);

private:
    LcdDriver _lcd; // Composition: Owns the driver
};

#endif
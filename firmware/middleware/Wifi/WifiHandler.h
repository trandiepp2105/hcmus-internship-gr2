#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include "WifiDriver.h"

/**
 * @class WifiHandler
 * @brief BSP/Middleware for WiFi management
 * Owns the WifiDriver and provides high-level WiFi API
 */
class WifiHandler {
public:
    WifiHandler();
    
    /**
     * @brief Initialize WiFi with captive portal
     * @param apName Access Point name for setup mode
     */
    void begin(const char* apName);
    
    bool isConnected();
    String getIP();
    void resetSettings();

private:
    WifiDriver _driver;
};

#endif

#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include "WifiDriver.h"
#include "../bsp_board.h"

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
    
    /**
     * @brief Call in loop to monitor connection and handle reconnect
     * Uses WIFI_RECONNECT_TIMEOUT_MS from bsp_board.h
     */
    void update();
    
    /**
     * @brief Manually start config portal (on-demand)
     */
    void startPortal();
    
    /**
     * @brief Stop config portal if running
     */
    void stopPortal();
    
    /**
     * @brief Check if portal is currently active
     */
    bool isPortalActive();
    
    bool isConnected();
    String getIP();
    void resetSettings();

private:
    WifiDriver _driver;
};

#endif

#ifndef WIFI_DRIVER_H
#define WIFI_DRIVER_H

#include <Arduino.h>
#include <WiFi.h>  // ESP32
#include <WiFiManager.h>

/**
 * @class WifiDriver
 * @brief Low-level driver for WiFi connection using WiFiManager (Captive Portal)
 */
class WifiDriver {
public:
    WifiDriver();
    
    /**
     * @brief Initialize WiFi with captive portal for config
     * @param apName Access Point name shown when no credentials saved
     * @param apPass Optional password for the AP
     * @return true if connected successfully
     */
    bool init(const char* apName, const char* apPass = NULL);
    
    /**
     * @brief Reset saved WiFi credentials (force re-config)
     */
    void resetSettings();
    
    /**
     * @brief Check connection status
     */
    bool isConnected();
    
    /**
     * @brief Get local IP address
     */
    String getIP();

private:
    WiFiManager _wm;
};

#endif

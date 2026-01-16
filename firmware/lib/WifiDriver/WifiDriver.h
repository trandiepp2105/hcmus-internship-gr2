#ifndef WIFI_DRIVER_H
#define WIFI_DRIVER_H

#include <Arduino.h>
#include <WiFi.h>  // ESP32
#include <WiFiManager.h>

/**
 * @class WifiDriver
 * @brief Low-level driver for WiFi connection using WiFiManager
 * Uses ESP32 WiFi Events instead of polling for efficiency
 */
class WifiDriver {
public:
    WifiDriver();
    
    /**
     * @brief Initialize WiFi with event handlers
     * @param apName Access Point name shown when no credentials saved
     * @param apPass Optional password for the AP
     * @return true if connected successfully
     */
    bool init(const char* apName, const char* apPass = NULL);
    
    /**
     * @brief Call in loop - only processes when flags are set (no polling)
     * @param timeoutMs Time before starting config portal
     */
    void update(unsigned long timeoutMs);
    
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
    const char* _apName;
    bool _portalActive;
    unsigned long _disconnectTime;
    
    // Event flags - set by ISR, processed in update()
    static volatile bool _eventDisconnected;
    static volatile bool _eventConnected;
    static WifiDriver* _instance;
    
    void setupEventHandlers();
    static void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
};

#endif

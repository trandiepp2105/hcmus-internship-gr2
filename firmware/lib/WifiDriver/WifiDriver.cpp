#include "WifiDriver.h"

WifiDriver::WifiDriver() {
}

bool WifiDriver::init(const char* apName, const char* apPass) {
    // WiFiManager handles:
    // 1. Try to connect with saved credentials
    // 2. If fails, start captive portal (AP mode) for user to enter new credentials
    // 3. Save to flash and connect
    
    Serial.println("[WiFi] Starting WiFiManager...");
    
    bool connected = false;
    if (apPass == NULL || strlen(apPass) == 0) {
        connected = _wm.autoConnect(apName);
    } else {
        connected = _wm.autoConnect(apName, apPass);
    }
    
    if (connected) {
        Serial.print("[WiFi] Connected! IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("[WiFi] Failed to connect. Restarting...");
        ESP.restart();
    }
    
    return connected;
}

void WifiDriver::resetSettings() {
    _wm.resetSettings();
    Serial.println("[WiFi] Settings reset.");
}

bool WifiDriver::isConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

String WifiDriver::getIP() {
    return WiFi.localIP().toString();
}

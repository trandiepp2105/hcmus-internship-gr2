#include "WifiDriver.h"

// Static member initialization
volatile bool WifiDriver::_eventDisconnected = false;
volatile bool WifiDriver::_eventConnected = false;
WifiDriver* WifiDriver::_instance = nullptr;

WifiDriver::WifiDriver() 
    : _apName(nullptr), _portalActive(false), _disconnectTime(0) {
    _instance = this;
}

void WifiDriver::setupEventHandlers() {
    // Register WiFi event callback - ESP32 will call this when WiFi status changes
    WiFi.onEvent(onWiFiEvent);
    Serial.println("[WiFi] Event handlers registered");
}

void WifiDriver::onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    // This is called from WiFi task - just set flags, don't do heavy work
    // Note: Event names changed in ESP32 Arduino Core 2.0+
    switch(event) {
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("[WiFi] EVENT: Disconnected!");
            _eventDisconnected = true;
            _eventConnected = false;
            break;
            
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("[WiFi] EVENT: Got IP: ");
            Serial.println(WiFi.localIP());
            _eventConnected = true;
            _eventDisconnected = false;
            break;
            
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("[WiFi] EVENT: Connected to AP");
            break;
            
        default:
            break;
    }
}

bool WifiDriver::init(const char* apName, const char* apPass) {
    _apName = apName;
    
    Serial.println("[WiFi] Starting WiFiManager...");
    
    // Setup event handlers BEFORE connecting
    setupEventHandlers();
    
    // Set callbacks
    _wm.setAPCallback([](WiFiManager* wm) {
        Serial.println("[WiFi] Config portal started");
    });
    
    _wm.setSaveConfigCallback([]() {
        Serial.println("[WiFi] New credentials saved");
    });
    
    bool connected = false;
    if (apPass == NULL || strlen(apPass) == 0) {
        connected = _wm.autoConnect(apName);
    } else {
        connected = _wm.autoConnect(apName, apPass);
    }
    
    if (connected) {
        Serial.print("[WiFi] Connected! IP: ");
        Serial.println(WiFi.localIP());
        _eventConnected = true;
    } else {
        Serial.println("[WiFi] Failed to connect after portal config");
    }
    
    return connected;
}

void WifiDriver::update(unsigned long timeoutMs) {
    // Process disconnect event (set by ISR)
    if (_eventDisconnected) {
        if (_disconnectTime == 0) {
            // Just disconnected - start timer
            _disconnectTime = millis();
            Serial.println("[WiFi] Starting reconnect timer...");
            
            // Try to reconnect immediately
            WiFi.reconnect();
        }
        
        // Check timeout
        if (_disconnectTime > 0 && (millis() - _disconnectTime) >= timeoutMs) {
            if (!_portalActive) {
                Serial.println("[WiFi] Timeout! Starting config portal...");
                startPortal();
            }
        }
    }
    
    // Process connect event (set by ISR)
    if (_eventConnected) {
        _disconnectTime = 0;
        _eventDisconnected = false;
        
        if (_portalActive) {
            stopPortal();
        }
    }
    
    // Process portal if active
    if (_portalActive) {
        _wm.process();
    }
}

void WifiDriver::startPortal() {
    if (_portalActive) return;
    
    Serial.println("[WiFi] Starting config portal (non-blocking)...");
    _wm.setConfigPortalBlocking(false);
    _wm.startConfigPortal(_apName);
    _portalActive = true;
}

void WifiDriver::stopPortal() {
    if (!_portalActive) return;
    
    Serial.println("[WiFi] Stopping config portal");
    _wm.stopConfigPortal();
    _portalActive = false;
}

bool WifiDriver::isPortalActive() {
    return _portalActive;
}

void WifiDriver::resetSettings() {
    _wm.resetSettings();
    Serial.println("[WiFi] Settings reset. Restart to reconfigure.");
}

bool WifiDriver::isConnected() {
    return _eventConnected && (WiFi.status() == WL_CONNECTED);
}

String WifiDriver::getIP() {
    return WiFi.localIP().toString();
}

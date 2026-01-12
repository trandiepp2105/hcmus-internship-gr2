#include "WifiHandler.h"

WifiHandler::WifiHandler() {
}

void WifiHandler::begin(const char* apName) {
    Serial.println("[WifiHandler] Initializing WiFi...");
    _driver.init(apName, NULL);
}

bool WifiHandler::isConnected() {
    return _driver.isConnected();
}

String WifiHandler::getIP() {
    return _driver.getIP();
}

void WifiHandler::resetSettings() {
    _driver.resetSettings();
}

#include "WifiHandler.h"

WifiHandler::WifiHandler() {
}

void WifiHandler::begin(const char* apName) {
    Serial.println("[WifiHandler] Initializing WiFi...");
    _driver.init(apName, NULL);
}

void WifiHandler::update() {
    _driver.update(WIFI_RECONNECT_TIMEOUT_MS);
}

void WifiHandler::startPortal() {
    _driver.startPortal();
}

void WifiHandler::stopPortal() {
    _driver.stopPortal();
}

bool WifiHandler::isPortalActive() {
    return _driver.isPortalActive();
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

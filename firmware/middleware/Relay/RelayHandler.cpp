#include "RelayHandler.h"

RelayHandler::RelayHandler(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin)
    : _driver(dataPin, clockPin, latchPin) {
}

void RelayHandler::begin() {
    _driver.init();
    Serial.println("[RelayHandler] Initialized 8-channel relay.");
}

void RelayHandler::setRelay(uint8_t relayNum, bool state) {
    if (relayNum > 7) {
        Serial.printf("[RelayHandler] Invalid relay number: %d\n", relayNum);
        return;
    }
    
    _driver.setBit(relayNum, state);
    Serial.printf("[RelayHandler] Relay %d = %s\n", relayNum, state ? "ON" : "OFF");
}

void RelayHandler::setAllRelays(uint8_t mask) {
    _driver.write(mask);
    Serial.printf("[RelayHandler] All relays set: 0x%02X\n", mask);
}

void RelayHandler::allOn() {
    _driver.setAll();
    Serial.println("[RelayHandler] All relays ON.");
}

void RelayHandler::allOff() {
    _driver.clearAll();
    Serial.println("[RelayHandler] All relays OFF.");
}

bool RelayHandler::getRelayState(uint8_t relayNum) {
    if (relayNum > 7) return false;
    return (_driver.getCurrentState() >> relayNum) & 0x01;
}

uint8_t RelayHandler::getCurrentState() {
    return _driver.getCurrentState();
}

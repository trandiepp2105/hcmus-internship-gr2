#include "ShiftRegisterDriver.h"

ShiftRegisterDriver::ShiftRegisterDriver(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin)
    : _dataPin(dataPin), _clockPin(clockPin), _latchPin(latchPin), _currentState(0) {
}

void ShiftRegisterDriver::init() {
    pinMode(_dataPin, OUTPUT);
    pinMode(_clockPin, OUTPUT);
    pinMode(_latchPin, OUTPUT);
    
    // Initialize all outputs to LOW
    clearAll();
    
    Serial.println("[ShiftReg] Initialized.");
}

void ShiftRegisterDriver::shiftOut() {
    // Hold latch LOW while shifting
    digitalWrite(_latchPin, LOW);
    
    // Use Arduino's built-in shiftOut function (MSBFIRST: Bit 7 goes to Q7)
    ::shiftOut(_dataPin, _clockPin, MSBFIRST, _currentState);
    
    // Latch: Transfer shift register to output register
    digitalWrite(_latchPin, HIGH);
}

void ShiftRegisterDriver::write(uint8_t data) {
    _currentState = data;
    shiftOut();
}

void ShiftRegisterDriver::setBit(uint8_t bit, bool state) {
    if (bit > 7) return;
    
    if (state) {
        _currentState |= (1 << bit);   // Set bit
    } else {
        _currentState &= ~(1 << bit);  // Clear bit
    }
    shiftOut();
}

void ShiftRegisterDriver::clearAll() {
    _currentState = 0x00;
    shiftOut();
}

void ShiftRegisterDriver::setAll() {
    _currentState = 0xFF;
    shiftOut();
}

uint8_t ShiftRegisterDriver::getCurrentState() {
    return _currentState;
}

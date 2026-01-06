#include "ButtonDriver.h"

ButtonDriver::ButtonDriver(uint8_t pin) 
    : _pin(pin), _currentState(false), _lastState(false), _lastDebounceTime(0) {}

void ButtonDriver::init() {
    pinMode(_pin, INPUT_PULLUP);
}

void ButtonDriver::update() {
    bool reading = (digitalRead(_pin) == LOW); 

    if (reading != _lastState) {
        _lastDebounceTime = millis();
    }

    if ((millis() - _lastDebounceTime) > _debounceDelay) {
        _currentState = reading;
    }
    _lastState = reading;
}

bool ButtonDriver::isPressed() {
    return _currentState;
}
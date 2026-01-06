#include "ButtonDriver.h"

InputManager::InputManager(uint8_t pin) {
    _pin = pin;
    _lastButtonState = HIGH; 
    _currentButtonState = HIGH;
    _lastDebounceTime = 0;
}

void InputManager::init() {
    pinMode(_pin, INPUT_PULLUP);
}

void InputManager::update() {
    int reading = digitalRead(_pin);

    if (reading != _lastButtonState) {
        _lastDebounceTime = millis();
    }

    if ((millis() - _lastDebounceTime) > _debounceDelay) {
        if (reading != _currentButtonState) {
            _currentButtonState = reading;
        }
    }
    _lastButtonState = reading;
}

bool InputManager::isClicked() {
    static bool handled = false; 
    if (_currentButtonState == LOW && !handled) {
        handled = true;
        return true; 
    } else if (_currentButtonState == HIGH) {
        handled = false; 
    }
    return false;
}
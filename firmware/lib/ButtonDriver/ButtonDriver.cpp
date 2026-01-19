#include "ButtonDriver.h"

// Button timing constants
#define DEBOUNCE_MS   50    // Minimum hold time for valid press
#define HOLD_MS       3000  // Hold time for long press

// Global pointers for ISR (ESP32 requires static function for ISR)
static ButtonDriver* btnAInstance = nullptr;
static ButtonDriver* btnBInstance = nullptr;
static ButtonDriver* btnCInstance = nullptr;

// Static ISR functions - triggered on RISING (release)
static void IRAM_ATTR btnA_ISR() {
    if (btnAInstance) btnAInstance->handleInterrupt();
}

static void IRAM_ATTR btnB_ISR() {
    if (btnBInstance) btnBInstance->handleInterrupt();
}

static void IRAM_ATTR btnC_ISR() {
    if (btnCInstance) btnCInstance->handleInterrupt();
}

ButtonDriver::ButtonDriver(uint8_t pin) 
    : _pin(pin), _pressed(false), _longPressed(false), 
      _pressStart(0), _lastInterruptTime(0) {
    // Register this instance for ISR
    if (btnAInstance == nullptr) {
        btnAInstance = this;
    } else if (btnBInstance == nullptr) {
        btnBInstance = this;
    } else if (btnCInstance == nullptr) {
        btnCInstance = this;
    }
}

void ButtonDriver::init() {
    pinMode(_pin, INPUT_PULLUP);
    
    // Attach interrupt on RISING edge (button released = HIGH)
    void (*isr)() = nullptr;
    if (this == btnAInstance) {
        isr = btnA_ISR;
    } else if (this == btnBInstance) {
        isr = btnB_ISR;
    } else if (this == btnCInstance) {
        isr = btnC_ISR;
    }
    
    if (isr) {
        attachInterrupt(digitalPinToInterrupt(_pin), isr, RISING);
    }
}

void IRAM_ATTR ButtonDriver::handleInterrupt() {
    uint32_t now = millis();
    
    // Debounce check
    if (now - _lastInterruptTime < 30) {
        return;  // Too fast, ignore
    }
    _lastInterruptTime = now;
    
    // Calculate hold time from press start
    if (_pressStart > 0) {
        uint32_t holdTime = now - _pressStart;
        
        if (holdTime >= HOLD_MS) {
            // Long press detected
            _longPressed = true;
            _pressed = false;
        }
        else if (holdTime >= DEBOUNCE_MS) {
            // Normal press detected
            _pressed = true;
            _longPressed = false;
        }
        // else: holdTime < DEBOUNCE_MS = noise, ignore
        
        _pressStart = 0;  // Reset for next press
    }
}

void ButtonDriver::updatePressState() {
    // Call this in main loop to detect press start
    bool currentState = digitalRead(_pin) == LOW;  // LOW = pressed
    
    if (currentState && _pressStart == 0) {
        // Button just pressed, record start time
        _pressStart = millis();
    }
}

bool ButtonDriver::checkClicked() {
    if (_pressed) {
        _pressed = false;
        return true;
    }
    return false;
}

bool ButtonDriver::checkLongPressed() {
    if (_longPressed) {
        _longPressed = false;
        return true;
    }
    return false;
}

bool ButtonDriver::isPressed() {
    return digitalRead(_pin) == LOW;
}
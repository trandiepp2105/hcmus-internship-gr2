#include "ButtonDriver.h"

// Global pointers for ISR (ESP32 requires static function for ISR)
static ButtonDriver* btnAInstance = nullptr;
static ButtonDriver* btnBInstance = nullptr;

// Timing measurement
volatile uint32_t lastButtonPressTime = 0;

// Static ISR functions
static void IRAM_ATTR btnA_ISR() {
    lastButtonPressTime = millis(); // Record ISR time
    if (btnAInstance) btnAInstance->handleInterrupt();
}

static void IRAM_ATTR btnB_ISR() {
    if (btnBInstance) btnBInstance->handleInterrupt();
}

ButtonDriver::ButtonDriver(uint8_t pin) 
    : _pin(pin), _pressed(false), _lastInterruptTime(0) {
    // Register this instance for ISR
    if (btnAInstance == nullptr) {
        btnAInstance = this;
    } else if (btnBInstance == nullptr) {
        btnBInstance = this;
    }
}

void ButtonDriver::init() {
    pinMode(_pin, INPUT_PULLUP);
    
    // Attach interrupt on FALLING edge (button pressed = LOW)
    void (*isr)() = nullptr;
    if (this == btnAInstance) {
        isr = btnA_ISR;
    } else if (this == btnBInstance) {
        isr = btnB_ISR;
    }
    
    if (isr) {
        attachInterrupt(digitalPinToInterrupt(_pin), isr, FALLING);
    }
}

void IRAM_ATTR ButtonDriver::handleInterrupt() {
    uint32_t now = millis();
    
    // Debounce: chi chap nhan neu da qua 150ms ke tu lan nhan truoc
    if (now - _lastInterruptTime > _debounceDelay) {
        _pressed = true;
        _lastInterruptTime = now;
    }
}

bool ButtonDriver::checkClicked() {
    // Kiem tra va reset flag (atomic operation)
    if (_pressed) {
        _pressed = false;  // Reset flag
        
        // Measure latency from ISR to here
        if (lastButtonPressTime > 0) {
            uint32_t latency = millis() - lastButtonPressTime;
            if (latency > 10) { // Only log if > 10ms
                Serial.printf("[BTN] Latency from ISR: %lu ms\n", latency);
            }
            lastButtonPressTime = 0;
        }
        
        return true;
    }
    return false;
}
#include "PotHandler.h"

PotHandler::PotHandler(uint8_t pin) 
    : _driver(pin) 
{
}

float PotHandler::getScaledValue(float minVal, float maxVal) {
    _driver.update(); // Update reading
    
    uint16_t raw = _driver.getValue(); // Read filtered value (0-4095 or 1023)
    
    // Check if 10bit or 12bit based on Driver?
    // PotDriver header says "0-1023" in comments but ESP32 is 12bit.
    // Assuming getValue returns raw ADC. If it is 10-bit (Arduino default), max is 1023.
    // If ESP32, standard analogRead is 0-4095. 
    // Let's assume 4095 for ESP32. If Driver scales to 1023, map needs adjustment.
    // But PotDriver implementation isn't shown fully, just header.
    // Safety: Use mapFloat with assumption of 0-4095 for now, or check Driver impl?
    // Step 416 viewed PotDriver.cpp: `_filteredValue = _alpha * raw + (1 - _alpha) * _filteredValue;`
    // `raw` comes from `analogRead(_pin)`.
    // On ESP32, `analogRead` is 12-bit (4095).
    
    return mapFloat((float)raw, 0.0f, 4095.0f, minVal, maxVal);
}

// Helper: Arduino map() uses long, here we want float
float PotHandler::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
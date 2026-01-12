#include "TempSensorHandler.h"

TempSensorHandler::TempSensorHandler(uint8_t pin) 
    : _driver(pin) 
{}

void TempSensorHandler::begin() {
    _driver.init();
}

float TempSensorHandler::getTemperature() {
    // Simple pass-through for now
    return _driver.readTemperature();
}

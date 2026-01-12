#include "TempSensorDriver.h"

TempSensorDriver::TempSensorDriver(uint8_t pin)
    : _pin(pin), _oneWire(pin), _sensors(&_oneWire) 
{
}

void TempSensorDriver::init() {
    _sensors.begin();
}

float TempSensorDriver::readTemperature() {
    _sensors.requestTemperatures(); // Request conversion
    float tempC = _sensors.getTempCByIndex(0); // We assume 1 sensor at index 0
    return tempC;
}

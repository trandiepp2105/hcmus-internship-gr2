#include "PotHandler.h"

PotHandler::PotHandler(PotDriver* driver) : _driver(driver) {}

float PotHandler::getScaledValue(float minVal, float maxVal) {
    _driver->update();
    uint16_t reading = _driver->getValue();
    return mapFloat((float)reading, 0.0f, 1023.0f, minVal, maxVal);
}

float PotHandler::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#include "PH_BSP.h"

PH_BSP::PH_BSP(STORAGE *storageDriver) : _storage(storageDriver)
{
    pinMode(PH_SENSOR_PIN, INPUT);
}

float PH_BSP::readPH()
{
    int raw = analogRead(PH_SENSOR_PIN);
    // Công thức chuyển đổi giả định cho ESP32 (12-bit ADC)
    return (float)raw * (14.0 / 4095.0);
}

void PH_BSP::saveData(float value)
{
    char msg[150];
    sprintf(msg, "{\"ph\":%.2f, \"ts\":%lu}", value, millis() / 1000);
    _storage->push(msg); // Gọi API của Driver
}
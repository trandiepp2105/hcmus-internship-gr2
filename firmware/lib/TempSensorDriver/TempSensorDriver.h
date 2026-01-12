#ifndef TEMP_SENSOR_DRIVER_H
#define TEMP_SENSOR_DRIVER_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/**
 * @class TempSensorDriver
 * @brief Wrapper for DallasTemperature (DS18B20) sensor
 */
class TempSensorDriver {
public:
    /**
     * @brief Constructor
     * @param pin GPIO Pin connected to DS18B20 Data Line
     */
    TempSensorDriver(uint8_t pin);

    void init();

    /**
     * @brief Reads temperature in Celsius
     * @return Temperature in C, or -127.0 if error
     */
    float readTemperature();

private:
    uint8_t _pin;
    OneWire _oneWire;
    DallasTemperature _sensors;
};

#endif

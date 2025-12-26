/**
 * @file temperature_sensor.h
 * @brief DS18B20 Temperature Sensor for pH ATC
 * @author PH_CONTROLLER Team
 */

#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Configuration
#define TEMP_SENSOR_PIN 4
#define TEMP_RESOLUTION 12
#define TEMP_SAMPLES_COUNT 3
#define TEMP_MIN_VALID -10.0
#define TEMP_MAX_VALID 50.0
#define TEMP_ERROR_VALUE -127.0

class TemperatureSensor
{
private:
    OneWire *oneWire;
    DallasTemperature *sensors;
    uint8_t sensorPin;
    DeviceAddress primarySensor;
    bool sensorFound;
    float lastValidTemp;

    float temperatureBuffer[5];
    uint8_t bufferIndex;
    bool bufferFilled;

    float movingAverageFilter(float temperature);

public:
    TemperatureSensor(uint8_t pin = TEMP_SENSOR_PIN);
    ~TemperatureSensor();

    bool begin();
    float readTemperature(bool filtered = true);
    float readTemperatureFahrenheit(bool filtered = true);
    bool isSensorConnected();
    float getLastValidTemperature();

    static float celsiusToFahrenheit(float celsius);
    void printSensorInfo();
};

#endif // TEMPERATURE_SENSOR_H
/**
 * @file temperature_sensor.cpp
 * @brief DS18B20 Temperature Sensor Implementation
 * @author PH_CONTROLLER Team
 */

#include "temperature_sensor.h"

TemperatureSensor::TemperatureSensor(uint8_t pin)
    : sensorPin(pin),
      sensorFound(false),
      lastValidTemp(25.0),
      bufferIndex(0),
      bufferFilled(false)
{

    oneWire = new OneWire(sensorPin);
    sensors = new DallasTemperature(oneWire);

    for (int i = 0; i < 5; i++)
    {
        temperatureBuffer[i] = 25.0;
    }
}

TemperatureSensor::~TemperatureSensor()
{
    delete sensors;
    delete oneWire;
}

bool TemperatureSensor::begin()
{
    Serial.println("[TempSensor] Initializing DS18B20...");
    Serial.printf("[TempSensor] Pin: GPIO %d\n", sensorPin);

    sensors->begin();
    uint8_t deviceCount = sensors->getDeviceCount();

    Serial.printf("[TempSensor] Found %d sensor(s)\n", deviceCount);

    if (deviceCount == 0)
    {
        Serial.println("[TempSensor] WARNING: No DS18B20 found!");
        Serial.println("[TempSensor] Check wiring:");
        Serial.println("  VCC -> 3.3V, GND -> GND");
        Serial.printf("  DATA -> GPIO %d (with 4.7k pullup)\n", sensorPin);
        return false;
    }

    if (sensors->getAddress(primarySensor, 0))
    {
        sensorFound = true;
        sensors->setResolution(primarySensor, TEMP_RESOLUTION);
        sensors->setWaitForConversion(false);

        Serial.print("[TempSensor] Address: ");
        for (uint8_t i = 0; i < 8; i++)
        {
            if (primarySensor[i] < 16)
                Serial.print("0");
            Serial.print(primarySensor[i], HEX);
            if (i < 7)
                Serial.print(":");
        }
        Serial.println();
        Serial.println("[TempSensor] Init complete!");
        return true;
    }

    Serial.println("[TempSensor] ERROR: Failed to get address");
    return false;
}

float TemperatureSensor::readTemperature(bool filtered)
{
    if (!sensorFound)
    {
        Serial.println("[TempSensor] Sensor not connected, using last valid");
        return lastValidTemp;
    }

    // Collect samples
    float samples[TEMP_SAMPLES_COUNT];
    uint8_t validCount = 0;

    for (int i = 0; i < TEMP_SAMPLES_COUNT; i++)
    {
        sensors->requestTemperatures();
        delay(750); // Wait for 12-bit conversion

        float temp = sensors->getTempC(primarySensor);

        // Validate reading
        if (temp != DEVICE_DISCONNECTED_C &&
            temp >= TEMP_MIN_VALID &&
            temp <= TEMP_MAX_VALID)
        {
            samples[validCount++] = temp;
        }

        delay(100);
    }

    if (validCount == 0)
    {
        Serial.println("[TempSensor] All readings invalid, using last valid");
        return lastValidTemp;
    }

    // Calculate average
    float sum = 0.0;
    for (int i = 0; i < validCount; i++)
    {
        sum += samples[i];
    }
    float avgTemp = sum / validCount;

    // Apply filter if requested
    if (filtered)
    {
        avgTemp = movingAverageFilter(avgTemp);
    }

    lastValidTemp = avgTemp;
    return avgTemp;
}

float TemperatureSensor::readTemperatureFahrenheit(bool filtered)
{
    float celsius = readTemperature(filtered);
    return celsiusToFahrenheit(celsius);
}

bool TemperatureSensor::isSensorConnected()
{
    if (!sensorFound)
        return false;
    return sensors->isConnected(primarySensor);
}

float TemperatureSensor::getLastValidTemperature()
{
    return lastValidTemp;
}

float TemperatureSensor::celsiusToFahrenheit(float celsius)
{
    return celsius * 9.0 / 5.0 + 32.0;
}

void TemperatureSensor::printSensorInfo()
{
    Serial.println("\n====================================");
    Serial.println("  DS18B20 Sensor Info");
    Serial.println("====================================");
    Serial.printf("Status:      %s\n", sensorFound ? "FOUND" : "NOT FOUND");
    Serial.printf("Connected:   %s\n", isSensorConnected() ? "YES" : "NO");
    Serial.printf("Last temp:   %.2f°C (%.2f°F)\n",
                  lastValidTemp, celsiusToFahrenheit(lastValidTemp));
    Serial.println("====================================\n");
}

float TemperatureSensor::movingAverageFilter(float temperature)
{
    temperatureBuffer[bufferIndex] = temperature;
    bufferIndex++;

    if (bufferIndex >= 5)
    {
        bufferIndex = 0;
        bufferFilled = true;
    }

    float sum = 0.0;
    int count = bufferFilled ? 5 : bufferIndex;

    for (int i = 0; i < count; i++)
    {
        sum += temperatureBuffer[i];
    }

    return sum / count;
}
/**
 * @file sensor_readings.cpp
 * @brief Implementation of pH sensor reading and filtering
 * @author PH_CONTROLLER Team
 * @date 2025-01-15
 */

#include "sensor_readings.h"
#include <algorithm>

// ============================================================================
// CONSTRUCTOR
// ============================================================================

SensorReadings::SensorReadings(uint8_t pin, uint16_t samples, uint16_t interval)
    : sensorPin(pin),
      samplesCount(samples),
      sampleInterval(interval),
      phSlope(DEFAULT_PH_SLOPE),
      phOffset(DEFAULT_PH_OFFSET),
      bufferIndex(0),
      bufferFilled(false)
{
    // Initialize voltage buffer to zero
    for (int i = 0; i < 20; i++)
    {
        voltageBuffer[i] = 0.0;
    }
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

bool SensorReadings::begin()
{
    // Configure ADC pin
    pinMode(sensorPin, INPUT);

    // Configure ADC resolution (ESP32 specific)
    analogReadResolution(ADC_RESOLUTION);

    // Configure ADC attenuation for 0-3.3V range (ESP32 specific)
    // ADC_11db allows measurement of voltages up to ~3.3V
    analogSetAttenuation(ADC_11db);

    // Perform initial readings to stabilize
    delay(100);
    for (int i = 0; i < 5; i++)
    {
        readRawADC();
        delay(10);
    }

    Serial.println("[SensorReadings] Initialization complete");
    Serial.printf("[SensorReadings] Pin: %d, Samples: %d, Interval: %dms\n",
                  sensorPin, samplesCount, sampleInterval);

    return true;
}

float SensorReadings::readVoltage()
{
    float samples[SAMPLES_COUNT];

    // Collect multiple samples
    for (int i = 0; i < samplesCount; i++)
    {
        uint16_t adcValue = readRawADC();
        samples[i] = adcToVoltage(adcValue);

        if (i < samplesCount - 1)
        {
            delay(sampleInterval);
        }
    }

    // Apply median filter to remove outliers
    float medianVoltage = medianFilter(samples, samplesCount);

    // Apply moving average filter for smoothing
    float filteredVoltage = movingAverageFilter(medianVoltage);

    return filteredVoltage;
}

float SensorReadings::readPH()
{
    float voltage = readVoltage();
    float pH = voltageToPH(voltage);

    // Constrain pH to valid range (0-14)
    if (pH < 0.0)
        pH = 0.0;
    if (pH > 14.0)
        pH = 14.0;

    return pH;
}

void SensorReadings::setCalibration(float slope, float offset)
{
    phSlope = slope;
    phOffset = offset;

    Serial.println("[SensorReadings] Calibration updated");
    Serial.printf("[SensorReadings] Slope: %.4f, Offset: %.4f\n", slope, offset);
}

void SensorReadings::getCalibration(float &slope, float &offset)
{
    slope = phSlope;
    offset = phOffset;
}

void SensorReadings::resetCalibration()
{
    phSlope = DEFAULT_PH_SLOPE;
    phOffset = DEFAULT_PH_OFFSET;

    Serial.println("[SensorReadings] Calibration reset to default");
    Serial.printf("[SensorReadings] Slope: %.4f, Offset: %.4f\n",
                  phSlope, phOffset);
}

uint16_t SensorReadings::getLastRawADC()
{
    return readRawADC();
}

float SensorReadings::voltageToPH(float voltage)
{
    // Convert voltage to millivolts
    float millivolts = voltage * 1000.0;

    // Apply linear calibration: pH = (mV - mV_at_pH7) / slope + 7
    // Simplified: pH = (mV / slope) + offset
    float pH = (millivolts / phSlope) + phOffset;

    return pH;
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

uint16_t SensorReadings::readRawADC()
{
    uint16_t adcValue = analogRead(sensorPin);

    // Ensure value is within valid range
    if (adcValue > ADC_MAX_VALUE)
    {
        adcValue = ADC_MAX_VALUE;
    }

    return adcValue;
}

float SensorReadings::adcToVoltage(uint16_t adcValue)
{
    // Convert ADC value to voltage: V = (ADC / ADC_MAX) * VREF
    float voltage = (float)adcValue / (float)ADC_MAX_VALUE * VREF;
    return voltage;
}

float SensorReadings::medianFilter(float *samples, uint16_t count)
{
    // Create a temporary array for sorting
    float tempArray[SAMPLES_COUNT];

    // Copy samples to temporary array
    for (int i = 0; i < count; i++)
    {
        tempArray[i] = samples[i];
    }

    // Sort the array using bubble sort (simple for small arrays)
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (tempArray[j] > tempArray[j + 1])
            {
                float temp = tempArray[j];
                tempArray[j] = tempArray[j + 1];
                tempArray[j + 1] = temp;
            }
        }
    }

    // Return median value
    if (count % 2 == 0)
    {
        // Even number of samples: average of two middle values
        return (tempArray[count / 2 - 1] + tempArray[count / 2]) / 2.0;
    }
    else
    {
        // Odd number of samples: middle value
        return tempArray[count / 2];
    }
}

float SensorReadings::movingAverageFilter(float voltage)
{
    // Add new voltage to buffer
    voltageBuffer[bufferIndex] = voltage;
    bufferIndex++;

    // Check if buffer is full
    if (bufferIndex >= 20)
    {
        bufferIndex = 0;
        bufferFilled = true;
    }

    // Calculate average
    float sum = 0.0;
    int count = bufferFilled ? 20 : bufferIndex;

    for (int i = 0; i < count; i++)
    {
        sum += voltageBuffer[i];
    }

    return sum / count;
}
/**
 * @file sensor_readings.h
 * @brief Header file for pH sensor reading and filtering
 * @author PH_CONTROLLER Team
 * @date 2025-01-15
 */

#ifndef SENSOR_READINGS_H
#define SENSOR_READINGS_H

#include <Arduino.h>

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

#define PH_SENSOR_PIN 34      // GPIO pin for pH sensor (ADC1_CH6)
#define ADC_RESOLUTION 12     // 12-bit ADC (0-4095)
#define ADC_MAX_VALUE 4095    // Maximum ADC value
#define VREF 3.3              // Reference voltage (ESP32)
#define SAMPLES_COUNT 10      // Number of samples for averaging
#define SAMPLE_INTERVAL_MS 50 // Interval between samples (ms)

// pH sensor calibration defaults
#define DEFAULT_PH_SLOPE -5.70 // Default slope (mV/pH)
#define DEFAULT_PH_OFFSET 2.5  // Default offset (pH at 0V)

// ============================================================================
// CLASS DEFINITION
// ============================================================================

/**
 * @class SensorReadings
 * @brief Manages pH sensor reading, filtering, and conversion
 */
class SensorReadings
{
private:
    uint8_t sensorPin;       // ADC pin number
    uint16_t samplesCount;   // Number of samples to average
    uint16_t sampleInterval; // Interval between samples (ms)
    float phSlope;           // Calibration slope
    float phOffset;          // Calibration offset

    // Internal buffers
    float voltageBuffer[20]; // Buffer for voltage samples
    uint16_t bufferIndex;    // Current buffer index
    bool bufferFilled;       // Buffer filled flag

    /**
     * @brief Read raw ADC value from sensor
     * @return Raw ADC value (0-4095)
     */
    uint16_t readRawADC();

    /**
     * @brief Convert ADC value to voltage
     * @param adcValue Raw ADC value
     * @return Voltage in volts
     */
    float adcToVoltage(uint16_t adcValue);

    /**
     * @brief Apply median filter to remove noise
     * @param samples Array of voltage samples
     * @param count Number of samples
     * @return Median filtered voltage
     */
    float medianFilter(float *samples, uint16_t count);

    /**
     * @brief Apply moving average filter
     * @param voltage Current voltage reading
     * @return Averaged voltage
     */
    float movingAverageFilter(float voltage);

public:
    /**
     * @brief Constructor
     * @param pin ADC pin number
     * @param samples Number of samples for averaging
     * @param interval Interval between samples (ms)
     */
    SensorReadings(uint8_t pin = PH_SENSOR_PIN,
                   uint16_t samples = SAMPLES_COUNT,
                   uint16_t interval = SAMPLE_INTERVAL_MS);

    /**
     * @brief Initialize sensor reading module
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Read and filter voltage from sensor
     * @return Filtered voltage in volts
     */
    float readVoltage();

    /**
     * @brief Read pH value from sensor
     * @return pH value (0-14)
     */
    float readPH();

    /**
     * @brief Set calibration parameters
     * @param slope Calibration slope (mV/pH)
     * @param offset Calibration offset (pH at 0V)
     */
    void setCalibration(float slope, float offset);

    /**
     * @brief Get current calibration parameters
     * @param slope Output parameter for slope
     * @param offset Output parameter for offset
     */
    void getCalibration(float &slope, float &offset);

    /**
     * @brief Reset calibration to default values
     */
    void resetCalibration();

    /**
     * @brief Get last raw ADC reading
     * @return Last ADC value
     */
    uint16_t getLastRawADC();

    /**
     * @brief Convert voltage to pH using calibration
     * @param voltage Voltage in volts
     * @return pH value
     */
    float voltageToPH(float voltage);
};

#endif // SENSOR_READINGS_H
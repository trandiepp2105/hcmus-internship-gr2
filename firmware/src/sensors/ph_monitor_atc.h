/**
 * @file ph_monitor_atc.h
 * @brief pH Monitor with Automatic Temperature Compensation (ATC)
 * @author PH_CONTROLLER Team
 * @date 2025-01-15
 */

#ifndef PH_MONITOR_ATC_H
#define PH_MONITOR_ATC_H

#include <Arduino.h>
#include "sensor_readings.h"
#include "ph_calibration.h"
#include "temperature_sensor.h" // Must be after sensor_readings.h

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

#define ATC_ENABLED true        // Enable/disable ATC
#define ATC_REFERENCE_TEMP 25.0 // Reference temperature (°C)
#define ATC_COEFFICIENT 0.003   // Temperature coefficient (~0.003 pH/°C)

// Nernst equation constants
#define NERNST_CONSTANT 59.16   // mV/pH at 25°C
#define ABSOLUTE_ZERO -273.15   // °C
#define REFERENCE_KELVIN 298.15 // 25°C in Kelvin

// pH buffer temperature coefficients (pH change per °C from 25°C)
#define BUFFER_PH4_COEFF 0.0001   // pH 4.0 buffer temp coefficient
#define BUFFER_PH7_COEFF 0.0002   // pH 7.0 buffer temp coefficient
#define BUFFER_PH10_COEFF -0.0011 // pH 10.0 buffer temp coefficient

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @struct PHReading
 * @brief Complete pH measurement with temperature
 */
struct PHReading
{
    float rawPH;         // pH without temperature compensation
    float compensatedPH; // pH with temperature compensation
    float temperature;   // Water temperature (°C)
    float voltage;       // Sensor voltage (V)
    uint16_t rawADC;     // Raw ADC value
    uint32_t timestamp;  // Reading timestamp (ms)
    bool valid;          // Reading validity flag
};

// ============================================================================
// CLASS DEFINITION
// ============================================================================

/**
 * @class PHMonitorATC
 * @brief pH monitoring system with automatic temperature compensation
 */
class PHMonitorATC
{
private:
    SensorReadings *phSensor;      // pH sensor instance
    TemperatureSensor *tempSensor; // Temperature sensor instance
    PHCalibration *calibration;    // Calibration data manager

    bool atcEnabled;       // ATC enable flag
    float referenceTemp;   // Reference temperature
    float tempCoefficient; // Temperature coefficient

    PHReading lastReading; // Last valid reading

    /**
     * @brief Calculate temperature-compensated pH using Nernst equation
     * @param rawPH Uncompensated pH value
     * @param temperature Current temperature (°C)
     * @return Temperature-compensated pH
     */
    float calculateNernstCompensation(float rawPH, float temperature);

    /**
     * @brief Simple linear temperature compensation
     * @param rawPH Uncompensated pH value
     * @param temperature Current temperature (°C)
     * @return Temperature-compensated pH
     */
    float calculateLinearCompensation(float rawPH, float temperature);

    /**
     * @brief Get temperature-adjusted calibration slope
     * @param temperature Current temperature (°C)
     * @return Adjusted slope value
     */
    float getTempAdjustedSlope(float temperature);

    /**
     * @brief Validate pH reading
     * @param reading pH reading structure
     * @return true if reading is valid
     */
    bool validateReading(const PHReading &reading);

public:
    /**
     * @brief Constructor
     * @param phPin GPIO pin for pH sensor
     * @param tempPin GPIO pin for temperature sensor
     */
    PHMonitorATC(uint8_t phPin = PH_SENSOR_PIN,
                 uint8_t tempPin = TEMP_SENSOR_PIN);

    /**
     * @brief Destructor
     */
    ~PHMonitorATC();

    /**
     * @brief Initialize pH monitoring system
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Read pH with automatic temperature compensation
     * @return Complete pH reading structure
     */
    PHReading readPH();

    /**
     * @brief Read pH without temperature compensation
     * @return pH reading structure (no compensation)
     */
    PHReading readPHRaw();

    /**
     * @brief Get last valid reading
     * @return Last pH reading
     */
    PHReading getLastReading();

    /**
     * @brief Enable/disable automatic temperature compensation
     * @param enable true to enable ATC
     */
    void enableATC(bool enable);

    /**
     * @brief Check if ATC is enabled
     * @return true if ATC is enabled
     */
    bool isATCEnabled();

    /**
     * @brief Set reference temperature for compensation
     * @param temp Reference temperature (°C)
     */
    void setReferenceTemperature(float temp);

    /**
     * @brief Get reference temperature
     * @return Reference temperature (°C)
     */
    float getReferenceTemperature();

    /**
     * @brief Set temperature compensation coefficient
     * @param coeff Temperature coefficient (pH/°C)
     */
    void setTempCoefficient(float coeff);

    /**
     * @brief Get temperature compensation coefficient
     * @return Temperature coefficient
     */
    float getTempCoefficient();

    /**
     * @brief Perform two-point calibration with temperature
     * @param ph4Voltage Voltage at pH 4.0 buffer
     * @param ph4Temp Temperature during pH 4.0 calibration
     * @param ph7Voltage Voltage at pH 7.0 buffer
     * @param ph7Temp Temperature during pH 7.0 calibration
     * @return true if calibration successful
     */
    bool calibrateTwoPoint(float ph4Voltage, float ph4Temp,
                           float ph7Voltage, float ph7Temp);

    /**
     * @brief Perform three-point calibration with temperature
     * @param ph4Voltage Voltage at pH 4.0 buffer
     * @param ph4Temp Temperature during pH 4.0 calibration
     * @param ph7Voltage Voltage at pH 7.0 buffer
     * @param ph7Temp Temperature during pH 7.0 calibration
     * @param ph10Voltage Voltage at pH 10.0 buffer
     * @param ph10Temp Temperature during pH 10.0 calibration
     * @return true if calibration successful
     */
    bool calibrateThreePoint(float ph4Voltage, float ph4Temp,
                             float ph7Voltage, float ph7Temp,
                             float ph10Voltage, float ph10Temp);

    /**
     * @brief Load calibration from EEPROM
     * @return true if successful
     */
    bool loadCalibration();

    /**
     * @brief Save calibration to EEPROM
     * @return true if successful
     */
    bool saveCalibration();

    /**
     * @brief Reset calibration to defaults
     * @return true if successful
     */
    bool resetCalibration();

    /**
     * @brief Get pH sensor instance
     * @return Pointer to SensorReadings
     */
    SensorReadings *getPHSensor();

    /**
     * @brief Get temperature sensor instance
     * @return Pointer to TemperatureSensor
     */
    TemperatureSensor *getTempSensor();

    /**
     * @brief Get calibration manager instance
     * @return Pointer to PHCalibration
     */
    PHCalibration *getCalibration();

    /**
     * @brief Print current reading with details
     * @param reading pH reading to print
     */
    void printReading(const PHReading &reading);

    /**
     * @brief Print system status
     */
    void printStatus();
};

#endif // PH_MONITOR_ATC_H
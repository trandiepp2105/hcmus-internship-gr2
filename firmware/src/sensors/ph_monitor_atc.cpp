/**
 * @file ph_monitor_atc.cpp
 * @brief Implementation of pH Monitor with ATC
 * @author PH_CONTROLLER Team
 * @date 2025-01-15
 */

#include "ph_monitor_atc.h"

// ============================================================================
// CONSTRUCTOR & DESTRUCTOR
// ============================================================================

PHMonitorATC::PHMonitorATC(uint8_t phPin, uint8_t tempPin)
    : atcEnabled(ATC_ENABLED),
      referenceTemp(ATC_REFERENCE_TEMP),
      tempCoefficient(ATC_COEFFICIENT)
{

    // Create sensor instances
    phSensor = new SensorReadings(phPin);
    tempSensor = new TemperatureSensor(tempPin);
    calibration = new PHCalibration();

    // Initialize last reading
    lastReading.rawPH = 0.0;
    lastReading.compensatedPH = 0.0;
    lastReading.temperature = referenceTemp;
    lastReading.voltage = 0.0;
    lastReading.rawADC = 0;
    lastReading.timestamp = 0;
    lastReading.valid = false;
}

PHMonitorATC::~PHMonitorATC()
{
    delete phSensor;
    delete tempSensor;
    delete calibration;
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

bool PHMonitorATC::begin()
{
    Serial.println("\n========================================");
    Serial.println("  pH Monitor with ATC - Initializing");
    Serial.println("========================================\n");

    // Initialize calibration (loads from EEPROM)
    if (!calibration->begin())
    {
        Serial.println("[PHMonitorATC] ERROR: Failed to initialize calibration");
        return false;
    }

    // Load and apply calibration to pH sensor
    float slope, offset;
    calibration->getCalibrationParams(slope, offset);
    phSensor->setCalibration(slope, offset);

    // Initialize pH sensor
    if (!phSensor->begin())
    {
        Serial.println("[PHMonitorATC] ERROR: Failed to initialize pH sensor");
        return false;
    }

    // Initialize temperature sensor
    if (!tempSensor->begin())
    {
        Serial.println("[PHMonitorATC] WARNING: Temperature sensor not found");
        Serial.printf("[PHMonitorATC] System will use reference temperature: %.2f°C\n",
                      referenceTemp);
        atcEnabled = false; // Disable ATC if no temp sensor
    }

    Serial.println("\n[PHMonitorATC] Initialization complete!");
    Serial.printf("[PHMonitorATC] ATC: %s\n", atcEnabled ? "ENABLED" : "DISABLED");
    Serial.printf("[PHMonitorATC] Reference temp: %.2f°C\n", referenceTemp);
    Serial.println("========================================\n");

    return true;
}

PHReading PHMonitorATC::readPH()
{
    PHReading reading;

    // Read temperature
    if (atcEnabled && tempSensor->isSensorConnected())
    {
        reading.temperature = tempSensor->readTemperature(true);
        if (reading.temperature == TEMP_ERROR_VALUE)
        {
            Serial.println("[PHMonitorATC] WARNING: Temperature read failed, using reference");
            reading.temperature = referenceTemp;
        }
    }
    else
    {
        reading.temperature = referenceTemp;
    }

    // Read voltage and ADC
    reading.voltage = phSensor->readVoltage();
    reading.rawADC = phSensor->getLastRawADC();

    // Calculate raw pH (no temperature compensation)
    reading.rawPH = phSensor->voltageToPH(reading.voltage);

    // Apply temperature compensation if enabled
    if (atcEnabled)
    {
        reading.compensatedPH = calculateNernstCompensation(reading.rawPH,
                                                            reading.temperature);
    }
    else
    {
        reading.compensatedPH = reading.rawPH;
    }

    // Set timestamp and validity
    reading.timestamp = millis();
    reading.valid = validateReading(reading);

    // Store as last reading if valid
    if (reading.valid)
    {
        lastReading = reading;
    }

    return reading;
}

PHReading PHMonitorATC::readPHRaw()
{
    bool atcState = atcEnabled;
    atcEnabled = false;
    PHReading reading = readPH();
    atcEnabled = atcState;
    return reading;
}

PHReading PHMonitorATC::getLastReading()
{
    return lastReading;
}

void PHMonitorATC::enableATC(bool enable)
{
    atcEnabled = enable;
    Serial.printf("[PHMonitorATC] ATC %s\n", enable ? "ENABLED" : "DISABLED");
}

bool PHMonitorATC::isATCEnabled()
{
    return atcEnabled;
}

void PHMonitorATC::setReferenceTemperature(float temp)
{
    if (temp >= 0.0 && temp <= 100.0)
    {
        referenceTemp = temp;
        Serial.printf("[PHMonitorATC] Reference temperature set to %.2f°C\n", temp);
    }
    else
    {
        Serial.println("[PHMonitorATC] ERROR: Invalid reference temperature");
    }
}

float PHMonitorATC::getReferenceTemperature()
{
    return referenceTemp;
}

void PHMonitorATC::setTempCoefficient(float coeff)
{
    tempCoefficient = coeff;
    Serial.printf("[PHMonitorATC] Temperature coefficient set to %.4f pH/°C\n", coeff);
}

float PHMonitorATC::getTempCoefficient()
{
    return tempCoefficient;
}

bool PHMonitorATC::calibrateTwoPoint(float ph4Voltage, float ph4Temp,
                                     float ph7Voltage, float ph7Temp)
{
    Serial.println("\n[PHMonitorATC] Starting two-point calibration with temperature...");

    // Adjust pH buffer values for temperature
    float ph4Adjusted = 4.0 + BUFFER_PH4_COEFF * (ph4Temp - 25.0);
    float ph7Adjusted = 7.0 + BUFFER_PH7_COEFF * (ph7Temp - 25.0);

    Serial.printf("[PHMonitorATC] pH 4.0 buffer at %.2f°C = %.3f\n", ph4Temp, ph4Adjusted);
    Serial.printf("[PHMonitorATC] pH 7.0 buffer at %.2f°C = %.3f\n", ph7Temp, ph7Adjusted);

    // Convert voltages to mV
    float v4_mv = ph4Voltage * 1000.0;
    float v7_mv = ph7Voltage * 1000.0;

    // Calculate slope: (V2 - V1) / (pH2 - pH1)
    float slope = (v7_mv - v4_mv) / (ph7Adjusted - ph4Adjusted);

    // Calculate offset
    float offset = ph7Adjusted - (v7_mv / slope);

    Serial.printf("[PHMonitorATC] Calculated slope: %.4f mV/pH\n", slope);
    Serial.printf("[PHMonitorATC] Calculated offset: %.4f pH\n", offset);

    // Set calibration
    calibration->setCalibrationParams(slope, offset);
    calibration->setCalibrationPoint(4.0, ph4Voltage);
    calibration->setCalibrationPoint(7.0, ph7Voltage);

    // Apply to pH sensor
    phSensor->setCalibration(slope, offset);

    Serial.println("[PHMonitorATC] Two-point calibration complete!");

    return true;
}

bool PHMonitorATC::calibrateThreePoint(float ph4Voltage, float ph4Temp,
                                       float ph7Voltage, float ph7Temp,
                                       float ph10Voltage, float ph10Temp)
{
    Serial.println("\n[PHMonitorATC] Starting three-point calibration with temperature...");

    // Adjust pH buffer values for temperature
    float ph4Adjusted = 4.0 + BUFFER_PH4_COEFF * (ph4Temp - 25.0);
    float ph7Adjusted = 7.0 + BUFFER_PH7_COEFF * (ph7Temp - 25.0);
    float ph10Adjusted = 10.0 + BUFFER_PH10_COEFF * (ph10Temp - 25.0);

    Serial.printf("[PHMonitorATC] pH 4.0 buffer at %.2f°C = %.3f\n", ph4Temp, ph4Adjusted);
    Serial.printf("[PHMonitorATC] pH 7.0 buffer at %.2f°C = %.3f\n", ph7Temp, ph7Adjusted);
    Serial.printf("[PHMonitorATC] pH 10.0 buffer at %.2f°C = %.3f\n", ph10Temp, ph10Adjusted);

    // Store calibration points
    calibration->setCalibrationPoint(4.0, ph4Voltage);
    calibration->setCalibrationPoint(7.0, ph7Voltage);
    calibration->setCalibrationPoint(10.0, ph10Voltage);

    // Calculate using least squares
    if (!calibration->calculateCalibration(3))
    {
        Serial.println("[PHMonitorATC] ERROR: Failed to calculate calibration");
        return false;
    }

    // Get calculated parameters
    float slope, offset;
    calibration->getCalibrationParams(slope, offset);

    // Apply to pH sensor
    phSensor->setCalibration(slope, offset);

    Serial.println("[PHMonitorATC] Three-point calibration complete!");

    return true;
}

bool PHMonitorATC::loadCalibration()
{
    if (calibration->loadCalibration())
    {
        float slope, offset;
        calibration->getCalibrationParams(slope, offset);
        phSensor->setCalibration(slope, offset);
        Serial.println("[PHMonitorATC] Calibration loaded from EEPROM");
        return true;
    }
    Serial.println("[PHMonitorATC] Failed to load calibration");
    return false;
}

bool PHMonitorATC::saveCalibration()
{
    if (calibration->saveCalibration())
    {
        Serial.println("[PHMonitorATC] Calibration saved to EEPROM");
        return true;
    }
    Serial.println("[PHMonitorATC] Failed to save calibration");
    return false;
}

bool PHMonitorATC::resetCalibration()
{
    if (calibration->resetCalibration())
    {
        float slope, offset;
        calibration->getCalibrationParams(slope, offset);
        phSensor->setCalibration(slope, offset);
        Serial.println("[PHMonitorATC] Calibration reset to defaults");
        return true;
    }
    return false;
}

SensorReadings *PHMonitorATC::getPHSensor()
{
    return phSensor;
}

TemperatureSensor *PHMonitorATC::getTempSensor()
{
    return tempSensor;
}

PHCalibration *PHMonitorATC::getCalibration()
{
    return calibration;
}

void PHMonitorATC::printReading(const PHReading &reading)
{
    Serial.println("\n========================================");
    Serial.println("  pH Reading with ATC");
    Serial.println("========================================");
    Serial.printf("Timestamp:       %lu ms\n", reading.timestamp);
    Serial.printf("Valid:           %s\n", reading.valid ? "YES" : "NO");
    Serial.println("----------------------------------------");
    Serial.printf("Raw ADC:         %d / %d\n", reading.rawADC, ADC_MAX_VALUE);
    Serial.printf("Voltage:         %.4f V\n", reading.voltage);
    Serial.printf("Temperature:     %.2f°C\n", reading.temperature);
    Serial.println("----------------------------------------");
    Serial.printf("Raw pH:          %.2f (no compensation)\n", reading.rawPH);
    Serial.printf("Compensated pH:  %.2f (with ATC)\n", reading.compensatedPH);
    Serial.printf("Difference:      %.2f pH units\n",
                  reading.compensatedPH - reading.rawPH);
    Serial.println("========================================\n");
}

void PHMonitorATC::printStatus()
{
    Serial.println("\n========================================");
    Serial.println("  pH Monitor ATC System Status");
    Serial.println("========================================");

    // ATC status
    Serial.printf("ATC Enabled:       %s\n", atcEnabled ? "YES" : "NO");
    Serial.printf("Reference Temp:    %.2f°C\n", referenceTemp);
    Serial.printf("Temp Coefficient:  %.4f pH/°C\n", tempCoefficient);
    Serial.println("----------------------------------------");

    // Temperature sensor
    Serial.printf("Temp Sensor:       %s\n",
                  tempSensor->isSensorConnected() ? "CONNECTED" : "DISCONNECTED");
    if (tempSensor->isSensorConnected())
    {
        Serial.printf("Last Temperature:  %.2f°C\n",
                      tempSensor->getLastValidTemperature());
    }
    Serial.println("----------------------------------------");

    // Calibration
    float slope, offset;
    calibration->getCalibrationParams(slope, offset);
    Serial.printf("Calibration Slope: %.4f mV/pH\n", slope);
    Serial.printf("Calibration Offset: %.4f pH\n", offset);
    Serial.println("----------------------------------------");

    // Last reading
    if (lastReading.valid)
    {
        Serial.println("Last Valid Reading:");
        Serial.printf("  pH (compensated): %.2f\n", lastReading.compensatedPH);
        Serial.printf("  Temperature:      %.2f°C\n", lastReading.temperature);
        Serial.printf("  Time ago:         %lu ms\n",
                      millis() - lastReading.timestamp);
    }
    else
    {
        Serial.println("No valid readings yet");
    }

    Serial.println("========================================\n");
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

float PHMonitorATC::calculateNernstCompensation(float rawPH, float temperature)
{
    // Nernst equation: E = E0 + (RT/nF) * ln([H+])
    // At different temperatures, the slope changes

    // Calculate temperature in Kelvin
    float tempK = temperature - ABSOLUTE_ZERO;

    // Calculate slope at current temperature
    // Slope(T) = (R * T * ln(10)) / F = 0.198 * T (mV/pH)
    // At 25°C (298.15K): Slope = 59.16 mV/pH
    float slopeAtTemp = (NERNST_CONSTANT / REFERENCE_KELVIN) * tempK;

    // Get calibration slope (at 25°C)
    float slope25C, offset;
    calibration->getCalibrationParams(slope25C, offset);

    // Adjust pH based on slope change
    float slopeRatio = slopeAtTemp / NERNST_CONSTANT;

    // Temperature-compensated pH
    // pH(T) = pH(25°C) + ΔpH
    // Where ΔpH accounts for slope change
    float deltaPH = (rawPH - 7.0) * (1.0 - slopeRatio);
    float compensatedPH = rawPH - deltaPH;

    return compensatedPH;
}

float PHMonitorATC::calculateLinearCompensation(float rawPH, float temperature)
{
    // Simple linear compensation: pH(25°C) = pH(T) + α(T - 25°C)
    float tempDiff = temperature - referenceTemp;
    float compensatedPH = rawPH + (tempCoefficient * tempDiff);

    return compensatedPH;
}

float PHMonitorATC::getTempAdjustedSlope(float temperature)
{
    float tempK = temperature - ABSOLUTE_ZERO;
    return (NERNST_CONSTANT / REFERENCE_KELVIN) * tempK;
}

bool PHMonitorATC::validateReading(const PHReading &reading)
{
    // Check pH range
    if (reading.compensatedPH < 0.0 || reading.compensatedPH > 14.0)
    {
        return false;
    }

    // Check temperature range
    if (reading.temperature < TEMP_MIN_VALID ||
        reading.temperature > TEMP_MAX_VALID)
    {
        return false;
    }

    // Check voltage range
    if (reading.voltage < 0.0 || reading.voltage > VREF)
    {
        return false;
    }

    return true;
}
/**
 * @file ph_calibration.cpp
 * @brief Implementation of pH calibration data management with EEPROM
 * @author PH_CONTROLLER Team
 * @date 2025-01-15
 */

#include "ph_calibration.h"

// ============================================================================
// CONSTRUCTOR
// ============================================================================

PHCalibration::PHCalibration() : isInitialized(false)
{
    // Initialize structure with defaults
    setDefaults();
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

bool PHCalibration::begin()
{
    // Initialize EEPROM with specified size
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        Serial.println("[PHCalibration] ERROR: Failed to initialize EEPROM");
        return false;
    }

    Serial.println("[PHCalibration] EEPROM initialized");
    Serial.printf("[PHCalibration] EEPROM Size: %d bytes\n", EEPROM_SIZE);

    // Try to load calibration from EEPROM
    if (loadCalibration())
    {
        Serial.println("[PHCalibration] Valid calibration loaded from EEPROM");
        isInitialized = true;
    }
    else
    {
        Serial.println("[PHCalibration] No valid calibration found, using defaults");
        setDefaults();
        saveCalibration(); // Save defaults to EEPROM
        isInitialized = true;
    }

    printCalibration();
    return true;
}

bool PHCalibration::loadCalibration()
{
    CalibrationData tempData;

    // Read data from EEPROM
    EEPROM.get(CALIBRATION_START_ADDRESS, tempData);

    // Verify magic number
    if (tempData.magic != CALIBRATION_MAGIC)
    {
        Serial.println("[PHCalibration] Invalid magic number in EEPROM");
        return false;
    }

    // Verify checksum
    if (!verifyChecksum(tempData))
    {
        Serial.println("[PHCalibration] Checksum verification failed");
        return false;
    }

    // Data is valid, copy to current calibration
    currentCalibration = tempData;

    return true;
}

bool PHCalibration::saveCalibration()
{
    // Update timestamp
    currentCalibration.timestamp = millis() / 1000; // Convert to seconds

    // Set magic number
    currentCalibration.magic = CALIBRATION_MAGIC;

    // Calculate and set checksum
    currentCalibration.checksum = calculateChecksum(currentCalibration);

    // Write to EEPROM
    EEPROM.put(CALIBRATION_START_ADDRESS, currentCalibration);

    // Commit changes (ESP32 specific)
    if (EEPROM.commit())
    {
        Serial.println("[PHCalibration] Calibration saved to EEPROM");
        return true;
    }
    else
    {
        Serial.println("[PHCalibration] ERROR: Failed to save calibration");
        return false;
    }
}

bool PHCalibration::resetCalibration()
{
    Serial.println("[PHCalibration] Resetting calibration to defaults...");

    setDefaults();

    if (saveCalibration())
    {
        Serial.println("[PHCalibration] Calibration reset successful");
        printCalibration();
        return true;
    }

    return false;
}

void PHCalibration::setCalibrationParams(float slope, float offset)
{
    currentCalibration.slope = slope;
    currentCalibration.offset = offset;

    Serial.printf("[PHCalibration] Parameters updated - Slope: %.4f, Offset: %.4f\n",
                  slope, offset);
}

void PHCalibration::getCalibrationParams(float &slope, float &offset)
{
    slope = currentCalibration.slope;
    offset = currentCalibration.offset;
}

bool PHCalibration::setCalibrationPoint(float phValue, float voltage)
{
    if (phValue == 4.0)
    {
        currentCalibration.ph4Voltage = voltage;
        Serial.printf("[PHCalibration] pH 4.0 point set: %.4f V\n", voltage);
        return true;
    }
    else if (phValue == 7.0)
    {
        currentCalibration.ph7Voltage = voltage;
        Serial.printf("[PHCalibration] pH 7.0 point set: %.4f V\n", voltage);
        return true;
    }
    else if (phValue == 10.0)
    {
        currentCalibration.ph10Voltage = voltage;
        Serial.printf("[PHCalibration] pH 10.0 point set: %.4f V\n", voltage);
        return true;
    }

    Serial.printf("[PHCalibration] ERROR: Invalid pH value %.1f (use 4.0, 7.0, or 10.0)\n",
                  phValue);
    return false;
}

bool PHCalibration::calculateCalibration(uint8_t usePoints)
{
    if (usePoints == 2)
    {
        // Two-point calibration using pH 4.0 and pH 7.0
        float voltage4 = currentCalibration.ph4Voltage;
        float voltage7 = currentCalibration.ph7Voltage;

        // Calculate slope: (V2 - V1) / (pH2 - pH1) in mV/pH
        float slope = ((voltage7 - voltage4) * 1000.0) / (7.0 - 4.0);

        // Calculate offset: pH = (V - V7) / slope + 7
        // Rearranged: offset = 7 - (V7 * 1000 / slope)
        float offset = 7.0 - ((voltage7 * 1000.0) / slope);

        currentCalibration.slope = slope;
        currentCalibration.offset = offset;
        currentCalibration.calibrationPoints = 2;

        Serial.println("[PHCalibration] Two-point calibration calculated");
        Serial.printf("[PHCalibration] Slope: %.4f mV/pH, Offset: %.4f pH\n",
                      slope, offset);

        return true;
    }
    else if (usePoints == 3)
    {
        // Three-point calibration using pH 4.0, 7.0, and 10.0
        // Using least squares linear regression

        float x[3] = {4.0, 7.0, 10.0}; // pH values
        float y[3] = {currentCalibration.ph4Voltage * 1000.0,
                      currentCalibration.ph7Voltage * 1000.0,
                      currentCalibration.ph10Voltage * 1000.0}; // Voltages in mV

        // Calculate means
        float meanX = (x[0] + x[1] + x[2]) / 3.0;
        float meanY = (y[0] + y[1] + y[2]) / 3.0;

        // Calculate slope: Σ((x - x̄)(y - ȳ)) / Σ((x - x̄)²)
        float numerator = 0.0;
        float denominator = 0.0;

        for (int i = 0; i < 3; i++)
        {
            numerator += (x[i] - meanX) * (y[i] - meanY);
            denominator += (x[i] - meanX) * (x[i] - meanX);
        }

        float slope = numerator / denominator;

        // Calculate offset: ȳ - slope * x̄
        float offset = 7.0 - (meanY / slope);

        currentCalibration.slope = slope;
        currentCalibration.offset = offset;
        currentCalibration.calibrationPoints = 3;

        Serial.println("[PHCalibration] Three-point calibration calculated");
        Serial.printf("[PHCalibration] Slope: %.4f mV/pH, Offset: %.4f pH\n",
                      slope, offset);

        return true;
    }

    Serial.printf("[PHCalibration] ERROR: Invalid number of points %d (use 2 or 3)\n",
                  usePoints);
    return false;
}

CalibrationData PHCalibration::getCalibrationData()
{
    return currentCalibration;
}

bool PHCalibration::isCalibrationValid()
{
    return (currentCalibration.magic == CALIBRATION_MAGIC &&
            verifyChecksum(currentCalibration));
}

void PHCalibration::printCalibration()
{
    Serial.println("\n========================================");
    Serial.println("  Current pH Calibration Data");
    Serial.println("========================================");
    Serial.printf("Magic:             0x%04X %s\n",
                  currentCalibration.magic,
                  currentCalibration.magic == CALIBRATION_MAGIC ? "[VALID]" : "[INVALID]");
    Serial.printf("Slope:             %.4f mV/pH\n", currentCalibration.slope);
    Serial.printf("Offset:            %.4f pH\n", currentCalibration.offset);
    Serial.printf("pH 4.0 Voltage:    %.4f V\n", currentCalibration.ph4Voltage);
    Serial.printf("pH 7.0 Voltage:    %.4f V\n", currentCalibration.ph7Voltage);
    Serial.printf("pH 10.0 Voltage:   %.4f V\n", currentCalibration.ph10Voltage);
    Serial.printf("Calibration Points: %d\n", currentCalibration.calibrationPoints);
    Serial.printf("Last Calibration:  %lu seconds ago\n",
                  (millis() / 1000) - currentCalibration.timestamp);
    Serial.printf("Checksum:          0x%04X %s\n",
                  currentCalibration.checksum,
                  verifyChecksum(currentCalibration) ? "[VALID]" : "[INVALID]");
    Serial.println("========================================\n");
}

uint32_t PHCalibration::getLastCalibrationTime()
{
    return currentCalibration.timestamp;
}

bool PHCalibration::clearEEPROM()
{
    Serial.println("[PHCalibration] Clearing EEPROM...");

    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i, 0xFF);
    }

    if (EEPROM.commit())
    {
        Serial.println("[PHCalibration] EEPROM cleared successfully");
        return true;
    }
    else
    {
        Serial.println("[PHCalibration] ERROR: Failed to clear EEPROM");
        return false;
    }
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

uint16_t PHCalibration::calculateChecksum(const CalibrationData &data)
{
    uint16_t checksum = 0;
    const uint8_t *ptr = (const uint8_t *)&data;

    // Calculate checksum for all bytes except the checksum field itself
    size_t size = sizeof(CalibrationData) - sizeof(data.checksum);

    for (size_t i = 0; i < size; i++)
    {
        checksum += ptr[i];
    }

    return checksum;
}

bool PHCalibration::verifyChecksum(const CalibrationData &data)
{
    uint16_t calculated = calculateChecksum(data);
    return (calculated == data.checksum);
}

void PHCalibration::setDefaults()
{
    currentCalibration.magic = CALIBRATION_MAGIC;
    currentCalibration.slope = DEFAULT_PH_SLOPE;
    currentCalibration.offset = DEFAULT_PH_OFFSET;
    currentCalibration.ph4Voltage = DEFAULT_PH4_VOLTAGE;
    currentCalibration.ph7Voltage = DEFAULT_PH7_VOLTAGE;
    currentCalibration.ph10Voltage = DEFAULT_PH10_VOLTAGE;
    currentCalibration.timestamp = 0;
    currentCalibration.calibrationPoints = 0;
    currentCalibration.checksum = 0;
}
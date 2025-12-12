/**
 * @file ph_calibration.h
 * @brief Header file for pH calibration data management with EEPROM
 * @author PH_CONTROLLER Team
 * @date 2025-01-15
 */

#ifndef PH_CALIBRATION_H
#define PH_CALIBRATION_H

#include <Arduino.h>
#include <EEPROM.h>

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

#define EEPROM_SIZE 512             // Total EEPROM size to allocate
#define CALIBRATION_START_ADDRESS 0 // Start address for calibration data
#define CALIBRATION_MAGIC 0xCAFE    // Magic number to verify valid data

// Default calibration values
#define DEFAULT_PH_SLOPE -5.70    // mV/pH
#define DEFAULT_PH_OFFSET 2.5     // pH at 0V
#define DEFAULT_PH4_VOLTAGE 2.03  // Voltage at pH 4.0 buffer
#define DEFAULT_PH7_VOLTAGE 1.50  // Voltage at pH 7.0 buffer
#define DEFAULT_PH10_VOLTAGE 0.97 // Voltage at pH 10.0 buffer

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @struct CalibrationData
 * @brief Structure to store pH calibration parameters
 */
struct CalibrationData
{
    uint16_t magic;            // Magic number for validation (0xCAFE)
    float slope;               // Calibration slope (mV/pH)
    float offset;              // Calibration offset (pH at 0V)
    float ph4Voltage;          // Measured voltage at pH 4.0
    float ph7Voltage;          // Measured voltage at pH 7.0
    float ph10Voltage;         // Measured voltage at pH 10.0
    uint32_t timestamp;        // Last calibration timestamp
    uint8_t calibrationPoints; // Number of calibration points used (0-3)
    uint16_t checksum;         // Checksum for data integrity
};

// ============================================================================
// CLASS DEFINITION
// ============================================================================

/**
 * @class PHCalibration
 * @brief Manages pH calibration data storage and retrieval from EEPROM
 */
class PHCalibration
{
private:
    CalibrationData currentCalibration;
    bool isInitialized;

    /**
     * @brief Calculate checksum for calibration data
     * @param data Calibration data structure
     * @return Calculated checksum
     */
    uint16_t calculateChecksum(const CalibrationData &data);

    /**
     * @brief Verify checksum of calibration data
     * @param data Calibration data structure
     * @return true if checksum is valid
     */
    bool verifyChecksum(const CalibrationData &data);

    /**
     * @brief Set default calibration values
     */
    void setDefaults();

public:
    /**
     * @brief Constructor
     */
    PHCalibration();

    /**
     * @brief Initialize EEPROM and load calibration data
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Load calibration data from EEPROM
     * @return true if valid calibration data loaded
     */
    bool loadCalibration();

    /**
     * @brief Save current calibration data to EEPROM
     * @return true if save successful
     */
    bool saveCalibration();

    /**
     * @brief Reset calibration to default values and save to EEPROM
     * @return true if reset successful
     */
    bool resetCalibration();

    /**
     * @brief Set calibration slope and offset
     * @param slope Calibration slope (mV/pH)
     * @param offset Calibration offset (pH at 0V)
     */
    void setCalibrationParams(float slope, float offset);

    /**
     * @brief Get current calibration slope and offset
     * @param slope Output parameter for slope
     * @param offset Output parameter for offset
     */
    void getCalibrationParams(float &slope, float &offset);

    /**
     * @brief Set measured voltage at specific pH point
     * @param phValue pH value (4.0, 7.0, or 10.0)
     * @param voltage Measured voltage at this pH
     * @return true if pH value is valid
     */
    bool setCalibrationPoint(float phValue, float voltage);

    /**
     * @brief Calculate slope and offset from calibration points
     * @param usePoints Number of points to use (2 or 3)
     * @return true if calculation successful
     */
    bool calculateCalibration(uint8_t usePoints = 2);

    /**
     * @brief Get calibration data structure
     * @return Current calibration data
     */
    CalibrationData getCalibrationData();

    /**
     * @brief Check if calibration data is valid
     * @return true if valid calibration exists
     */
    bool isCalibrationValid();

    /**
     * @brief Print calibration data to Serial
     */
    void printCalibration();

    /**
     * @brief Get last calibration timestamp
     * @return Unix timestamp of last calibration
     */
    uint32_t getLastCalibrationTime();

    /**
     * @brief Clear all EEPROM data
     * @return true if clear successful
     */
    bool clearEEPROM();
};

#endif // PH_CALIBRATION_H
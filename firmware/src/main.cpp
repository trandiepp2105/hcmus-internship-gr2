/**
 * @file main.cpp
 * @brief Main program for testing pH sensor readings
 * @author PH_CONTROLLER Team
 * @date 2025-01-15
 */

#include <Arduino.h>
#include "sensors/sensor_readings.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

SensorReadings phSensor;

// ============================================================================
// SETUP FUNCTION
// ============================================================================

void setup()
{
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("  PH_CONTROLLER - Sensor Readings Test");
    Serial.println("========================================");
    Serial.println();

    // Initialize pH sensor
    if (phSensor.begin())
    {
        Serial.println("[SETUP] pH Sensor initialized successfully");
    }
    else
    {
        Serial.println("[SETUP] ERROR: Failed to initialize pH sensor");
        while (1)
        {
            delay(1000);
        }
    }

    // Display current calibration
    float slope, offset;
    phSensor.getCalibration(slope, offset);
    Serial.printf("[SETUP] Current Calibration - Slope: %.4f, Offset: %.4f\n",
                  slope, offset);

    Serial.println();
    Serial.println("Starting pH monitoring...");
    Serial.println("========================================");
    Serial.println();
}

// ============================================================================
// LOOP FUNCTION
// ============================================================================

void loop()
{
    // Read raw ADC value
    uint16_t rawADC = phSensor.getLastRawADC();

    // Read filtered voltage
    float voltage = phSensor.readVoltage();

    // Read pH value
    float pH = phSensor.readPH();

    // Display readings
    Serial.println("----------------------------------------");
    Serial.printf("Raw ADC:  %d / %d\n", rawADC, ADC_MAX_VALUE);
    Serial.printf("Voltage:  %.4f V\n", voltage);
    Serial.printf("pH Value: %.2f\n", pH);
    Serial.println("----------------------------------------");
    Serial.println();

    // Wait before next reading
    delay(2000);
}

// ============================================================================
// OPTIONAL: Command handler for testing calibration
// ============================================================================

/**
 * Uncomment this section if you want to test calibration via Serial commands
 * Commands:
 *   - "CAL slope offset" : Set calibration (e.g., "CAL -5.70 2.5")
 *   - "RESET"           : Reset to default calibration
 *   - "GET"             : Get current calibration
 */

/*
void handleSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command.startsWith("CAL ")) {
            // Parse slope and offset
            int firstSpace = command.indexOf(' ');
            int secondSpace = command.indexOf(' ', firstSpace + 1);

            if (secondSpace > 0) {
                float slope = command.substring(firstSpace + 1, secondSpace).toFloat();
                float offset = command.substring(secondSpace + 1).toFloat();

                phSensor.setCalibration(slope, offset);
                Serial.println("[CMD] Calibration updated");
            }
        }
        else if (command == "RESET") {
            phSensor.resetCalibration();
            Serial.println("[CMD] Calibration reset");
        }
        else if (command == "GET") {
            float slope, offset;
            phSensor.getCalibration(slope, offset);
            Serial.printf("[CMD] Slope: %.4f, Offset: %.4f\n", slope, offset);
        }
    }
}

// Add this to loop():
// handleSerialCommands();
*/
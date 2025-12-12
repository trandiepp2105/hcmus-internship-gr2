/**
 * @file main.cpp
 * @brief Main program for testing pH calibration EEPROM functions
 * @author PH_CONTROLLER Team
 * @date 2025-01-15
 */

#include <Arduino.h>
#include "sensors/ph_calibration.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

PHCalibration calibration;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void printMenu();
void handleSerialCommands();
void testCalibrationFlow();

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
    Serial.println("  PH_CONTROLLER - Calibration EEPROM Test");
    Serial.println("========================================");
    Serial.println();

    // Initialize calibration module
    if (calibration.begin())
    {
        Serial.println("[SETUP] Calibration module initialized successfully");
    }
    else
    {
        Serial.println("[SETUP] ERROR: Failed to initialize calibration module");
        while (1)
        {
            delay(1000);
        }
    }

    Serial.println();
    printMenu();
}

// ============================================================================
// LOOP FUNCTION
// ============================================================================

void loop()
{
    handleSerialCommands();
    delay(100);
}

// ============================================================================
// MENU DISPLAY
// ============================================================================

void printMenu()
{
    Serial.println("\n========================================");
    Serial.println("  Available Commands");
    Serial.println("========================================");
    Serial.println("1. LOAD      - Load calibration from EEPROM");
    Serial.println("2. SAVE      - Save current calibration to EEPROM");
    Serial.println("3. RESET     - Reset calibration to defaults");
    Serial.println("4. PRINT     - Print current calibration data");
    Serial.println("5. SET s o   - Set slope and offset (e.g., SET -5.70 2.5)");
    Serial.println("6. POINT p v - Set calibration point (e.g., POINT 7.0 1.50)");
    Serial.println("7. CALC n    - Calculate calibration (n=2 or 3 points)");
    Serial.println("8. CLEAR     - Clear entire EEPROM");
    Serial.println("9. TEST      - Run automatic calibration test");
    Serial.println("10. MENU     - Show this menu");
    Serial.println("========================================\n");
}

// ============================================================================
// COMMAND HANDLER
// ============================================================================

void handleSerialCommands()
{
    if (Serial.available() > 0)
    {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toUpperCase();

        Serial.printf("\n[CMD] Received: %s\n", command.c_str());

        // Parse command
        if (command == "LOAD")
        {
            if (calibration.loadCalibration())
            {
                Serial.println("[CMD] Calibration loaded successfully");
                calibration.printCalibration();
            }
            else
            {
                Serial.println("[CMD] Failed to load calibration");
            }
        }
        else if (command == "SAVE")
        {
            if (calibration.saveCalibration())
            {
                Serial.println("[CMD] Calibration saved successfully");
            }
            else
            {
                Serial.println("[CMD] Failed to save calibration");
            }
        }
        else if (command == "RESET")
        {
            if (calibration.resetCalibration())
            {
                Serial.println("[CMD] Calibration reset successfully");
            }
            else
            {
                Serial.println("[CMD] Failed to reset calibration");
            }
        }
        else if (command == "PRINT")
        {
            calibration.printCalibration();
        }
        else if (command.startsWith("SET "))
        {
            // Parse slope and offset
            int firstSpace = command.indexOf(' ');
            int secondSpace = command.indexOf(' ', firstSpace + 1);

            if (secondSpace > 0)
            {
                float slope = command.substring(firstSpace + 1, secondSpace).toFloat();
                float offset = command.substring(secondSpace + 1).toFloat();

                calibration.setCalibrationParams(slope, offset);
                Serial.println("[CMD] Calibration parameters updated");
            }
            else
            {
                Serial.println("[CMD] ERROR: Invalid format. Use: SET slope offset");
            }
        }
        else if (command.startsWith("POINT "))
        {
            // Parse pH value and voltage
            int firstSpace = command.indexOf(' ');
            int secondSpace = command.indexOf(' ', firstSpace + 1);

            if (secondSpace > 0)
            {
                float phValue = command.substring(firstSpace + 1, secondSpace).toFloat();
                float voltage = command.substring(secondSpace + 1).toFloat();

                if (calibration.setCalibrationPoint(phValue, voltage))
                {
                    Serial.println("[CMD] Calibration point set successfully");
                }
                else
                {
                    Serial.println("[CMD] ERROR: Failed to set calibration point");
                }
            }
            else
            {
                Serial.println("[CMD] ERROR: Invalid format. Use: POINT pH voltage");
            }
        }
        else if (command.startsWith("CALC "))
        {
            uint8_t points = command.substring(5).toInt();

            if (calibration.calculateCalibration(points))
            {
                Serial.println("[CMD] Calibration calculated successfully");
                calibration.printCalibration();
            }
            else
            {
                Serial.println("[CMD] ERROR: Failed to calculate calibration");
            }
        }
        else if (command == "CLEAR")
        {
            Serial.println("[CMD] WARNING: This will erase all EEPROM data!");
            Serial.println("[CMD] Type CONFIRM to proceed:");

            delay(5000); // Wait for confirmation

            if (Serial.available())
            {
                String confirm = Serial.readStringUntil('\n');
                confirm.trim();
                confirm.toUpperCase();

                if (confirm == "CONFIRM")
                {
                    if (calibration.clearEEPROM())
                    {
                        Serial.println("[CMD] EEPROM cleared successfully");
                    }
                }
                else
                {
                    Serial.println("[CMD] Clear operation cancelled");
                }
            }
        }
        else if (command == "TEST")
        {
            testCalibrationFlow();
        }
        else if (command == "MENU")
        {
            printMenu();
        }
        else
        {
            Serial.println("[CMD] Unknown command. Type MENU for help.");
        }

        Serial.println();
    }
}

// ============================================================================
// TEST FUNCTION
// ============================================================================

void testCalibrationFlow()
{
    Serial.println("\n========================================");
    Serial.println("  Running Automatic Calibration Test");
    Serial.println("========================================\n");

    // Step 1: Reset calibration
    Serial.println("[TEST] Step 1: Resetting calibration...");
    calibration.resetCalibration();
    delay(1000);

    // Step 2: Simulate pH 4.0 measurement
    Serial.println("[TEST] Step 2: Simulating pH 4.0 buffer measurement...");
    calibration.setCalibrationPoint(4.0, 2.03);
    delay(500);

    // Step 3: Simulate pH 7.0 measurement
    Serial.println("[TEST] Step 3: Simulating pH 7.0 buffer measurement...");
    calibration.setCalibrationPoint(7.0, 1.50);
    delay(500);

    // Step 4: Calculate two-point calibration
    Serial.println("[TEST] Step 4: Calculating two-point calibration...");
    calibration.calculateCalibration(2);
    delay(500);

    // Step 5: Save calibration
    Serial.println("[TEST] Step 5: Saving calibration to EEPROM...");
    calibration.saveCalibration();
    delay(500);

    // Step 6: Verify by loading
    Serial.println("[TEST] Step 6: Verifying by loading from EEPROM...");
    calibration.loadCalibration();

    Serial.println("\n[TEST] Test completed successfully!");
    calibration.printCalibration();

    Serial.println("========================================\n");
}
/**
 * @file main.cpp
 * @brief Complete pH Monitor with ATC System
 * @author PH_CONTROLLER Team
 * @date 2025-01-15
 */

#include <Arduino.h>
#include "sensors/ph_monitor_atc.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

PHMonitorATC phMonitor;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void printMainMenu();
void printCalibrationMenu();
void handleSerialCommands();
void displayReading(const PHReading &reading);
void runCalibrationWizard();

// ============================================================================
// SETUP FUNCTION
// ============================================================================

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n\n");
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║   PH_CONTROLLER v1.0                  ║");
  Serial.println("║   pH Monitor with ATC System          ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();

  // Initialize pH monitoring system
  if (!phMonitor.begin())
  {
    Serial.println("\n⚠️  INITIALIZATION FAILED!");
    Serial.println("System cannot start. Check connections.");
    while (1)
    {
      delay(1000);
    }
  }

  Serial.println("✅ System ready!");
  printMainMenu();

  Serial.println("\n🔬 Starting continuous pH monitoring...\n");
}

// ============================================================================
// LOOP FUNCTION
// ============================================================================

void loop()
{
  // Handle serial commands
  handleSerialCommands();

  // Read pH with ATC
  PHReading reading = phMonitor.readPH();

  // Display reading
  displayReading(reading);

  // Wait before next reading
  delay(3000);
}

// ============================================================================
// MENU DISPLAYS
// ============================================================================

void printMainMenu()
{
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║          MAIN MENU                    ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println("📊 MONITORING:");
  Serial.println("  READ       - Single pH reading");
  Serial.println("  READRAW    - Read without ATC");
  Serial.println("  STATUS     - System status");
  Serial.println();
  Serial.println("🔧 ATC CONTROL:");
  Serial.println("  ATC ON     - Enable ATC");
  Serial.println("  ATC OFF    - Disable ATC");
  Serial.println("  TEMP n     - Set reference temp (°C)");
  Serial.println("  COEFF n    - Set temp coefficient");
  Serial.println();
  Serial.println("📐 CALIBRATION:");
  Serial.println("  CAL        - Calibration wizard");
  Serial.println("  CAL2       - Two-point calibration");
  Serial.println("  CAL3       - Three-point calibration");
  Serial.println("  LOAD       - Load from EEPROM");
  Serial.println("  SAVE       - Save to EEPROM");
  Serial.println("  RESET      - Reset to defaults");
  Serial.println();
  Serial.println("ℹ️  OTHER:");
  Serial.println("  MENU       - Show this menu");
  Serial.println("  HELP       - Calibration help");
  Serial.println("════════════════════════════════════════\n");
}

void printCalibrationMenu()
{
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║      CALIBRATION WIZARD               ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();
  Serial.println("📋 Required Materials:");
  Serial.println("  • pH 4.0 buffer solution");
  Serial.println("  • pH 7.0 buffer solution");
  Serial.println("  • pH 10.0 buffer (optional)");
  Serial.println("  • Clean water for rinsing");
  Serial.println();
  Serial.println("⚠️  Important:");
  Serial.println("  • Rinse probe between buffers");
  Serial.println("  • Wait for stable readings");
  Serial.println("  • Record temperature of each buffer");
  Serial.println();
  Serial.println("Starting calibration process...\n");
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

    Serial.printf("\n💬 Command: %s\n", command.c_str());
    Serial.println("────────────────────────────────────────");

    // Monitoring commands
    if (command == "READ")
    {
      PHReading reading = phMonitor.readPH();
      phMonitor.printReading(reading);
    }
    else if (command == "READRAW")
    {
      PHReading reading = phMonitor.readPHRaw();
      Serial.println("[Raw reading - no temperature compensation]");
      phMonitor.printReading(reading);
    }
    else if (command == "STATUS")
    {
      phMonitor.printStatus();
    }

    // ATC commands
    else if (command == "ATC ON")
    {
      phMonitor.enableATC(true);
    }
    else if (command == "ATC OFF")
    {
      phMonitor.enableATC(false);
    }
    else if (command.startsWith("TEMP "))
    {
      float temp = command.substring(5).toFloat();
      phMonitor.setReferenceTemperature(temp);
    }
    else if (command.startsWith("COEFF "))
    {
      float coeff = command.substring(6).toFloat();
      phMonitor.setTempCoefficient(coeff);
    }

    // Calibration commands
    else if (command == "CAL")
    {
      runCalibrationWizard();
    }
    else if (command == "CAL2")
    {
      Serial.println("📐 Two-Point Calibration");
      Serial.println("\n📍 Step 1: pH 4.0 Buffer");
      Serial.println("Place probe in pH 4.0 buffer");
      Serial.println("Enter: voltage temperature");
      Serial.println("Example: 2.03 25.5");
    }
    else if (command == "CAL3")
    {
      Serial.println("📐 Three-Point Calibration");
      Serial.println("Not yet implemented in wizard");
      Serial.println("Use manual calibration instead");
    }
    else if (command == "LOAD")
    {
      phMonitor.loadCalibration();
    }
    else if (command == "SAVE")
    {
      phMonitor.saveCalibration();
    }
    else if (command == "RESET")
    {
      Serial.println("⚠️  Are you sure? Type CONFIRM to reset:");
      delay(5000);
      if (Serial.available())
      {
        String confirm = Serial.readStringUntil('\n');
        confirm.trim();
        confirm.toUpperCase();
        if (confirm == "CONFIRM")
        {
          phMonitor.resetCalibration();
        }
        else
        {
          Serial.println("❌ Reset cancelled");
        }
      }
    }

    // Help and menu
    else if (command == "MENU")
    {
      printMainMenu();
    }
    else if (command == "HELP")
    {
      Serial.println("\n📖 CALIBRATION HELP");
      Serial.println("════════════════════════════════════════");
      Serial.println("1. Rinse probe with distilled water");
      Serial.println("2. Place in pH 7.0 buffer, wait 30s");
      Serial.println("3. Record voltage and temperature");
      Serial.println("4. Rinse probe again");
      Serial.println("5. Place in pH 4.0 buffer, wait 30s");
      Serial.println("6. Record voltage and temperature");
      Serial.println("7. Calculate calibration");
      Serial.println("8. Save to EEPROM");
      Serial.println();
      Serial.println("📊 Buffer temperatures affect pH:");
      Serial.println("  pH 7.0 @ 25°C = 7.00");
      Serial.println("  pH 7.0 @ 20°C = 7.00 (stable)");
      Serial.println("  pH 4.0 @ 25°C = 4.00");
      Serial.println("  pH 4.0 @ 20°C = 4.00 (stable)");
      Serial.println("════════════════════════════════════════\n");
    }

    else
    {
      Serial.println("❌ Unknown command");
      Serial.println("Type MENU for available commands\n");
    }
  }
}

// ============================================================================
// DISPLAY FUNCTION
// ============================================================================

void displayReading(const PHReading &reading)
{
  // Status indicator
  String status;
  String emoji;

  if (!reading.valid)
  {
    emoji = "⚠️";
    status = "INVALID";
  }
  else if (reading.compensatedPH < 4.0)
  {
    emoji = "🔴";
    status = "VERY ACIDIC";
  }
  else if (reading.compensatedPH < 6.5)
  {
    emoji = "🟡";
    status = "ACIDIC";
  }
  else if (reading.compensatedPH < 7.5)
  {
    emoji = "🟢";
    status = "NEUTRAL";
  }
  else if (reading.compensatedPH < 9.0)
  {
    emoji = "🔵";
    status = "BASIC";
  }
  else
  {
    emoji = "🟣";
    status = "VERY BASIC";
  }

  Serial.println("╔════════════════════════════════════════╗");
  Serial.printf("║  %s %-32s ║\n", emoji.c_str(), status.c_str());
  Serial.println("╠════════════════════════════════════════╣");
  Serial.printf("║  pH (ATC):     %19.2f ║\n", reading.compensatedPH);
  Serial.printf("║  pH (Raw):     %19.2f ║\n", reading.rawPH);
  Serial.printf("║  Temperature:  %15.2f °C ║\n", reading.temperature);
  Serial.println("╠════════════════════════════════════════╣");
  Serial.printf("║  Voltage:      %16.4f V ║\n", reading.voltage);
  Serial.printf("║  ADC:          %12d / 4095 ║\n", reading.rawADC);
  Serial.printf("║  ATC:          %19s ║\n",
                phMonitor.isATCEnabled() ? "ON" : "OFF");
  Serial.println("╚════════════════════════════════════════╝\n");
}

// ============================================================================
// CALIBRATION WIZARD
// ============================================================================

void runCalibrationWizard()
{
  printCalibrationMenu();

  Serial.println("🧪 Step 1: pH 7.0 Buffer");
  Serial.println("────────────────────────────────────────");
  Serial.println("1. Rinse probe with distilled water");
  Serial.println("2. Place probe in pH 7.0 buffer");
  Serial.println("3. Wait 30 seconds for stabilization");
  Serial.println("4. Press ENTER when ready...");

  while (!Serial.available())
  {
    delay(100);
  }
  Serial.readStringUntil('\n');

  Serial.println("\n📊 Reading pH 7.0 buffer...");
  delay(2000);

  PHReading reading7 = phMonitor.readPHRaw();
  float v7 = reading7.voltage;
  float t7 = reading7.temperature;

  Serial.printf("✓ Voltage: %.4f V\n", v7);
  Serial.printf("✓ Temperature: %.2f °C\n", t7);

  Serial.println("\n🧪 Step 2: pH 4.0 Buffer");
  Serial.println("────────────────────────────────────────");
  Serial.println("1. Remove probe and rinse thoroughly");
  Serial.println("2. Place probe in pH 4.0 buffer");
  Serial.println("3. Wait 30 seconds for stabilization");
  Serial.println("4. Press ENTER when ready...");

  while (!Serial.available())
  {
    delay(100);
  }
  Serial.readStringUntil('\n');

  Serial.println("\n📊 Reading pH 4.0 buffer...");
  delay(2000);

  PHReading reading4 = phMonitor.readPHRaw();
  float v4 = reading4.voltage;
  float t4 = reading4.temperature;

  Serial.printf("✓ Voltage: %.4f V\n", v4);
  Serial.printf("✓ Temperature: %.2f °C\n", t4);

  Serial.println("\n🔬 Calculating calibration...");
  delay(1000);

  if (phMonitor.calibrateTwoPoint(v4, t4, v7, t7))
  {
    Serial.println("✅ Calibration successful!");
    Serial.println("\n💾 Save to EEPROM? (Y/N)");

    delay(5000);
    if (Serial.available())
    {
      String response = Serial.readStringUntil('\n');
      response.trim();
      response.toUpperCase();

      if (response == "Y" || response == "YES")
      {
        phMonitor.saveCalibration();
        Serial.println("✅ Saved!");
      }
      else
      {
        Serial.println("⚠️  Not saved (will be lost on restart)");
      }
    }
  }
  else
  {
    Serial.println("❌ Calibration failed!");
  }

  Serial.println("\n────────────────────────────────────────");
  Serial.println("Calibration complete. Returning to monitoring...\n");
}
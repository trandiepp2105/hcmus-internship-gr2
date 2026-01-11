#include <Arduino.h>
#include <Wire.h>

// Layer 1: Hardware Definitions
#include "../middleware/bsp_board.h"

// Layer 2: Middleware/BSP modules (Each owns a Driver)
#include "../middleware/Lcd/LcdHandler.h"
#include "../middleware/Button/ButtonHandler.h"
#include "../middleware/Potentiometer/PotHandler.h"
#include "../lib/Storage/Storage.h" // Utils/System Driver

// Layer 3: Application
#include "App/Controllers/PhController.h"

// --- Global Objects ---

// 1. Storage (System)
Storage storage;

// 2. BSP Modules (Hardware Wrappers)
LcdHandler lcd; // Owns LcdDriver, pin config internal/default
ButtonHandler btnA(PIN_BTN_A); // Owns ButtonDriver
ButtonHandler btnB(PIN_BTN_B);
PotHandler potUpper(PIN_POT_UPPER); // Owns PotDriver
PotHandler potLower(PIN_POT_LOWER);

// 3. Application
PhController app(&storage, 
                 // &ioExpander, // Disabled
                 &btnA, 
                 &btnB, 
                 &lcd, 
                 &potUpper, 
                 &potLower);

void setup() {
    // 1. Init System Basics
    Serial.begin(115200);
    Serial.println("\n--- pH Controller Firmware Starting ---");


    // 2. Init Middleware/BSP
    if (!storage.begin("ph_config", false)) {
         Serial.println("Storage Init Failed");
    }
    
    lcd.begin(); // Init LCD Driver
    btnA.begin(); // Init Button Driver
    btnB.begin();
    // Pot doesn't need begin currently

    // 3. Init App
    app.begin();
}

void loop() {
    // Application Loop
    app.update();
}

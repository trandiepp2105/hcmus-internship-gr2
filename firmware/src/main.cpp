#include <Arduino.h>
#include <Wire.h> // Include Wire explicitly for I2C

// --- Libraries & Drivers ---
#include <Storage.h>
// #include <IOExpanderBSP.h> // Disabled per user request

// --- Middleware ---
#include "../middleware/bsp_board.h" // Pin definitions
#include "../middleware/Button/ButtonHandler.h"
#include "../middleware/Lcd/LcdHandler.h"
#include "../middleware/Potentiometer/PotHandler.h"

// --- Application Logic ---
#include "App/Controllers/PhController.h"

// --- Global Objects ---
Storage storage;
// IOExpanderBSP ioExpander;

LcdDriver lcdDriver(LCD_ADDR, LCD_COLS, LCD_ROWS);
LcdHandler lcdHandler(&lcdDriver);

ButtonDriver btnDriverA(PIN_BTN_A);
ButtonHandler btnHandlerA(&btnDriverA);

ButtonDriver btnDriverB(PIN_BTN_B);
ButtonHandler btnHandlerB(&btnDriverB);

PotDriver potDriverUpper(PIN_POT_UPPER);
PotHandler potHandlerUpper(&potDriverUpper);

PotDriver potDriverLower(PIN_POT_LOWER);
PotHandler potHandlerLower(&potDriverLower);

// Main Controller
PhController* controller;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- pH Controller Firmware Starting ---");

    // 1. Init I2C (Important fix for LCD)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    
    // 2. Init Storage
    if (!storage.begin("ph_config", false)) {
        Serial.println("Storage Init Failed!");
    }

    // 3. Init Hardware Drivers
    lcdDriver.init();
    // btnDriverA.begin(); // Assuming simple GPIO, driver might not need specific begin if init in constructor, checking...
    // Let's assume Drivers define pins in constructor or need init. ButtonDriver likely sets pinMode.
    // Checking ButtonDriver implementation previously showed... actually I pulled updates, let's assume standard behavior.
    // Just to be safe, standard Arduino setup for pins if drivers don't do it implicitly.
    pinMode(PIN_BTN_A, INPUT_PULLUP); // Redundant if Driver does it but safe
    pinMode(PIN_BTN_B, INPUT_PULLUP);

    // 4. Init Controller
    controller = new PhController(&storage, 
                                  // &ioExpander,
                                  &btnHandlerA, 
                                  &btnHandlerB, 
                                  &lcdHandler, 
                                  &potHandlerUpper, 
                                  &potHandlerLower);
                                  
    controller->begin();
    
    Serial.println("System Initialized.");
}

void loop() {
    // Main Application Loop
    if (controller) {
        controller->update();
    }
    
    // Simple delay to prevent watchdog starvation and debouncing ease (controller handles logic timing)
    delay(50); 
}
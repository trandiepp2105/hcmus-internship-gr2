#include <Arduino.h>
#include "../middleware/bsp_board.h"
#include "ButtonDriver.h"
#include "ButtonHandler.h"
#include "LcdDriver.h"
#include "LcdHandler.h"
#include "PotDriver.h"
#include "PotHandler.h"
#include "SystemController.h"

// --- 1. Driver Layer Allocation (Hardware Abstraction) ---
// Initialize drivers with pin definitions from bsp_board.h
ButtonDriver switchDrv(PIN_BTN_SWITCH);
ButtonDriver calibDrv(PIN_BTN_CALIB); 
PotDriver    potUpperDrv(PIN_POT_UPPER);
PotDriver    potLowerDrv(PIN_POT_LOWER);
LcdDriver    lcdDrv(0x27, 16, 2);

// --- 2. Middleware Layer Allocation (Signal Processing) ---
// Inject Drivers into Handlers
ButtonHandler switchHandler(&switchDrv);
ButtonHandler calibHandler(&calibDrv); 
PotHandler    potUpperHandler(&potUpperDrv);
PotHandler    potLowerHandler(&potLowerDrv);
LcdHandler    lcdHandler(&lcdDrv);

// --- 3. Application Layer Allocation (Business Logic) ---
// Inject Handlers into the Main Controller
// Note: Based on your latest code, SystemController currently takes 3 arguments
SystemController controller(&switchHandler, &lcdHandler, &potUpperHandler, &potLowerHandler);

void setup() {
    // Optional: Serial for debugging
    Serial.begin(115200);

    // Initialize Hardware (Drivers)
    switchDrv.init();
    calibDrv.init();
    potUpperDrv.init();
    potLowerDrv.init();
    lcdDrv.init();

    // Initialize System Logic
    controller.init();
}

void loop() {
    // Update System Logic Loop
    controller.update();
}
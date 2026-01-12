#include <Arduino.h>
#include <Wire.h>

// Layer 1: Hardware Definitions
#include "../middleware/bsp_board.h"

// Layer 2: Middleware/BSP modules (Each owns a Driver)
#include "../middleware/Lcd/LcdHandler.h"
#include "../middleware/Button/ButtonHandler.h"
#include "../middleware/Potentiometer/PotHandler.h"
#include "../middleware/Sensor/TempSensorHandler.h"
#include "../middleware/Wifi/WifiHandler.h"
#include "../middleware/Mqtt/MqttHandler.h"
#include "../middleware/Relay/RelayHandler.h"
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
TempSensorHandler tempSensor(PIN_TEMP_SENSOR);
WifiHandler wifi;
MqttHandler mqtt(&storage);
RelayHandler relays(PIN_SR_DATA, PIN_SR_CLOCK, PIN_SR_LATCH);

// 3. Application
PhController app(&storage, 
                 // &ioExpander, // Disabled
                 &btnA, 
                 &btnB, 
                 &lcd, 
                 &potUpper, 
                 &potLower,
                 &tempSensor,
                 &relays);

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
    tempSensor.begin(); // Init DS18B20
    relays.begin(); // Init 74HC595 Relay Controller
    // Pot doesn't need begin currently

    // 3. Init Network
    wifi.begin(WIFI_AP_NAME);
    mqtt.begin(MQTT_SERVER, MQTT_PORT);
    mqtt.checkAndProvision(TB_DEVICE_NAME, TB_PROVISION_KEY, TB_PROVISION_SECRET);

    // 4. Init App
    app.begin();
}

void loop() {
    // Network maintenance
    mqtt.update(TB_DEVICE_NAME);
    
    // Application Loop
    app.update();
}

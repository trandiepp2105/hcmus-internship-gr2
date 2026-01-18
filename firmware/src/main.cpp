#include <Arduino.h>
#include <Wire.h>

// Layer 1: Hardware Definitions
#include "../middleware/bsp_board.h"

// Layer 2: Middleware/BSP modules (Each owns a Driver)
#include "../middleware/Tft/TftHandler.h"
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
TftHandler tft; // Owns TftDriver, SPI display
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
                 &tft,
                 &wifi,
                 &potUpper, 
                 &potLower,
                 &tempSensor,
                 &relays,
                 &mqtt
                );

void setup() {
    // 1. Init System Basics
    Serial.begin(115200);
    Serial.println("\n--- pH Controller Firmware Starting ---");

    // 2. Init Middleware/BSP
    if (!storage.begin("ph_config", false)) {
         Serial.println("Storage Init Failed");
    }
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    // I2C Scanner - Find devices
    Serial.println("[I2C] Scanning for devices...");
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[I2C] Found device at 0x%02X\n", addr);
        }
    }
    Serial.println("[I2C] Scan complete.");

    tft.begin(); // Init TFT Display
    btnA.begin(); // Init Button Driver
    btnB.begin();
    tempSensor.begin(); // Init DS18B20
    relays.begin(); // Init 74HC595 Relay Controller
    
    // 3. Init Network
    // wifi.resetSettings();
    wifi.begin(WIFI_AP_NAME);
    mqtt.begin(MQTT_SERVER, MQTT_PORT);
    mqtt.checkAndProvision(TB_DEVICE_NAME, TB_PROVISION_KEY, TB_PROVISION_SECRET);

    // 4. Init App
    app.begin();
}

void loop() {
    static uint32_t lastMqttUpdate = 0;
    static uint32_t lastWifiUpdate = 0;
    uint32_t loopStart = millis();
    uint32_t now = millis();
    
    // PRIORITY 1: Handle button inputs FIRST for instant response
    // Button uses interrupt, but logic executes here
    uint32_t t1 = millis();
    app.handleInputs();
    uint32_t handleInputsTime = millis() - t1;
    
    // PRIORITY 2: WiFi maintenance (every 500ms max)
    t1 = millis();
    uint32_t wifiTime = 0;
    if (now - lastWifiUpdate > 500) {
        wifi.update();
        wifiTime = millis() - t1;
        lastWifiUpdate = now;
    }
    
    // PRIORITY 3: Network maintenance (every 1000ms max to avoid blocking)
    t1 = millis();
    uint32_t mqttTime = 0;
    if (now - lastMqttUpdate > 1000) {
        mqtt.update(TB_DEVICE_NAME);
        mqttTime = millis() - t1;
        lastMqttUpdate = now;
    }
    
    // PRIORITY 4: Application Loop (sensors, control logic, display)
    // Note: handleInputs() is called above, so it runs in update() too but that's OK
    t1 = millis();
    app.update();
    uint32_t appUpdateTime = millis() - t1;
    
    uint32_t totalLoopTime = millis() - loopStart;
    
    // Log if any component takes too long
    if (totalLoopTime > 100) {
        Serial.printf("[LOOP] Total:%lu ms (handleInputs:%lu, wifi:%lu, mqtt:%lu, app:%lu)\n",
                      totalLoopTime, handleInputsTime, wifiTime, mqttTime, appUpdateTime);
    }
}

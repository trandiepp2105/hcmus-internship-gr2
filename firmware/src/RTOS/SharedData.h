#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

// Reuse SystemMode from PhContext
#include "../App/Models/PhContext.h"

// ==================== BUTTON ENUMS ====================

enum ButtonId {
    BUTTON_A = 0,
    BUTTON_B,
    BUTTON_C
};

enum ButtonAction {
    BUTTON_PRESS = 0,
    BUTTON_LONG_PRESS
};

// ==================== SHARED DATA STRUCTURES ====================

struct SharedSensorData {
    float ph;
    float temp;
    uint32_t lastUpdate;
};

struct SharedConfigData {
    float phUpper;
    float phLower;
    float calibSlope;
    float calibIntercept;
    SystemMode mode;
};

struct SharedRelayData {
    bool relay1;
    bool relay2;
    bool relay3;
    bool relay4;
};

struct ButtonEvent {
    ButtonId button;
    ButtonAction action;
    uint32_t timestamp;
};

// ==================== GLOBAL INSTANCES ====================

extern SharedSensorData g_sensorData;
extern SharedConfigData g_configData;
extern SharedRelayData g_relayData;

// ==================== SYNCHRONIZATION PRIMITIVES ====================

extern SemaphoreHandle_t g_sensorDataMutex;
extern SemaphoreHandle_t g_configDataMutex;
extern SemaphoreHandle_t g_relayDataMutex;
extern SemaphoreHandle_t g_displayUpdateSem;
extern QueueHandle_t g_buttonQueue;

// ==================== TASK HANDLES ====================

extern TaskHandle_t g_buttonTaskHandle;
extern TaskHandle_t g_displayTaskHandle;
extern TaskHandle_t g_sensorTaskHandle;
extern TaskHandle_t g_controlTaskHandle;
extern TaskHandle_t g_networkTaskHandle;

// ==================== HELPER FUNCTIONS ====================

void initRTOS();

// Thread-safe getters/setters
void setPhData(float ph, float temp);
void getPhData(float* ph, float* temp);

void setSystemMode(SystemMode mode);
SystemMode getSystemMode();

void setRelayState(uint8_t relay, bool state);
bool getRelayState(uint8_t relay);

#endif // SHARED_DATA_H

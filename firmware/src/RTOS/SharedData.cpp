#include "SharedData.h"

// ==================== GLOBAL INSTANCES ====================

SharedSensorData g_sensorData = {7.0f, 25.0f, 0};
SharedConfigData g_configData = {8.5f, 6.5f, 1.0f, 0.0f, MODE_AUTO};
SharedRelayData g_relayData = {false, false, false, false};

// ==================== SYNCHRONIZATION PRIMITIVES ====================

SemaphoreHandle_t g_sensorDataMutex = NULL;
SemaphoreHandle_t g_configDataMutex = NULL;
SemaphoreHandle_t g_relayDataMutex = NULL;
SemaphoreHandle_t g_displayUpdateSem = NULL;
QueueHandle_t g_buttonQueue = NULL;

// ==================== TASK HANDLES ====================

TaskHandle_t g_buttonTaskHandle = NULL;
TaskHandle_t g_displayTaskHandle = NULL;
TaskHandle_t g_sensorTaskHandle = NULL;
TaskHandle_t g_controlTaskHandle = NULL;
TaskHandle_t g_networkTaskHandle = NULL;

// ==================== INITIALIZATION ====================

void initRTOS() {
    Serial.println("[RTOS] Initializing synchronization primitives...");
    
    g_sensorDataMutex = xSemaphoreCreateMutex();
    g_configDataMutex = xSemaphoreCreateMutex();
    g_relayDataMutex = xSemaphoreCreateMutex();
    g_displayUpdateSem = xSemaphoreCreateBinary();
    g_buttonQueue = xQueueCreate(10, sizeof(ButtonEvent));
    
    if (g_sensorDataMutex && g_configDataMutex && g_relayDataMutex && 
        g_displayUpdateSem && g_buttonQueue) {
        Serial.println("[RTOS] Primitives created OK");
    } else {
        Serial.println("[RTOS] ERROR: Failed to create primitives!");
    }
}

// ==================== THREAD-SAFE HELPERS ====================

void setPhData(float ph, float temp) {
    if (xSemaphoreTake(g_sensorDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_sensorData.ph = ph;
        g_sensorData.temp = temp;
        g_sensorData.lastUpdate = millis();
        xSemaphoreGive(g_sensorDataMutex);
    }
}

void getPhData(float* ph, float* temp) {
    if (xSemaphoreTake(g_sensorDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        *ph = g_sensorData.ph;
        *temp = g_sensorData.temp;
        xSemaphoreGive(g_sensorDataMutex);
    }
}

void setSystemMode(SystemMode mode) {
    if (xSemaphoreTake(g_configDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        g_configData.mode = mode;
        xSemaphoreGive(g_configDataMutex);
    }
}

SystemMode getSystemMode() {
    SystemMode mode = MODE_AUTO;
    if (xSemaphoreTake(g_configDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        mode = g_configData.mode;
        xSemaphoreGive(g_configDataMutex);
    }
    return mode;
}

void setRelayState(uint8_t relay, bool state) {
    if (xSemaphoreTake(g_relayDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        switch(relay) {
            case 1: g_relayData.relay1 = state; break;
            case 2: g_relayData.relay2 = state; break;
            case 3: g_relayData.relay3 = state; break;
            case 4: g_relayData.relay4 = state; break;
        }
        xSemaphoreGive(g_relayDataMutex);
    }
}

bool getRelayState(uint8_t relay) {
    bool state = false;
    if (xSemaphoreTake(g_relayDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        switch(relay) {
            case 1: state = g_relayData.relay1; break;
            case 2: state = g_relayData.relay2; break;
            case 3: state = g_relayData.relay3; break;
            case 4: state = g_relayData.relay4; break;
        }
        xSemaphoreGive(g_relayDataMutex);
    }
    return state;
}

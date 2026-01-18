#include "ButtonTask.h"
#include "../../middleware/Button/ButtonHandler.h"

// Button handlers from main.cpp
extern ButtonHandler btnMode;
extern ButtonHandler btnThreshold;
extern ButtonHandler btnCalib;

void buttonTask(void* param) {
    Serial.println("[ButtonTask] Started (Priority 5)");
    
    TickType_t lastWakeTime = xTaskGetTickCount();
    
    while(1) {
        // Check MODE button
        if (btnMode.checkClicked()) {
            ButtonEvent event = {BUTTON_A, BUTTON_PRESS, millis()};
            if (xQueueSend(g_buttonQueue, &event, 0) == pdTRUE) {
                Serial.println("[ButtonTask] MODE button -> queued");
            }
        }
        
        // Check THRESHOLD button
        if (btnThreshold.checkClicked()) {
            ButtonEvent event = {BUTTON_B, BUTTON_PRESS, millis()};
            if (xQueueSend(g_buttonQueue, &event, 0) == pdTRUE) {
                Serial.println("[ButtonTask] THRESHOLD button -> queued");
            }
        }
        
        // Check CALIB button
        if (btnCalib.checkClicked()) {
            ButtonEvent event = {BUTTON_C, BUTTON_PRESS, millis()};
            if (xQueueSend(g_buttonQueue, &event, 0) == pdTRUE) {
                Serial.println("[ButtonTask] CALIB button -> queued");
            }
        }
        
        // 10ms cycle = 100Hz polling
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10));
    }
}

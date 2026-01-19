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
        // Update all buttons (track press start time)
        btnMode.update();
        btnThreshold.update();
        btnCalib.update();
        
        // ===== MODE Button =====
        if (btnMode.checkLongPressed()) {
            ButtonEvent event = {BUTTON_A, BUTTON_LONG_PRESS, millis()};
            if (xQueueSend(g_buttonQueue, &event, 0) == pdTRUE) {
                Serial.println("[ButtonTask] MODE LONG PRESS -> FACTORY RESET");
            }
        }
        else if (btnMode.checkClicked()) {
            ButtonEvent event = {BUTTON_A, BUTTON_PRESS, millis()};
            if (xQueueSend(g_buttonQueue, &event, 0) == pdTRUE) {
                Serial.println("[ButtonTask] MODE PRESS");
            }
        }
        
        // ===== THRESHOLD Button =====
        if (btnThreshold.checkClicked()) {
            ButtonEvent event = {BUTTON_B, BUTTON_PRESS, millis()};
            if (xQueueSend(g_buttonQueue, &event, 0) == pdTRUE) {
                Serial.println("[ButtonTask] THRESHOLD PRESS");
            }
        }
        
        // ===== CALIB Button =====
        if (btnCalib.checkClicked()) {
            ButtonEvent event = {BUTTON_C, BUTTON_PRESS, millis()};
            if (xQueueSend(g_buttonQueue, &event, 0) == pdTRUE) {
                Serial.println("[ButtonTask] CALIB PRESS");
            }
        }
        
        // 10ms cycle = 100Hz polling
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10));
    }
}

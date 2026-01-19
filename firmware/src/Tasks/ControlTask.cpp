#include "ControlTask.h"
#include "../App/Controllers/PhController.h"

// App controller from main.cpp
extern PhController app;

void controlTask(void* param) {
    Serial.println("[ControlTask] Started (Priority 4)");
    
    ButtonEvent event;
    
    while(1) {
        // Wait for button event (block up to 50ms then check again)
        if (xQueueReceive(g_buttonQueue, &event, pdMS_TO_TICKS(50)) == pdTRUE) {
            uint32_t latency = millis() - event.timestamp;
            const char* btnName = (event.button == 0) ? "MODE" : 
                                  (event.button == 1) ? "THRESHOLD" : "CALIB";
            
            // Check if long press (factory reset)
            if (event.action == BUTTON_LONG_PRESS && event.button == 0) {
                Serial.printf("[ControlTask] %s LONG PRESS -> FACTORY RESET!\n", btnName);
                app.factoryReset();
            } else {
                Serial.printf("[ControlTask] %s button (latency %lu ms)\n", btnName, latency);
                app.handleButtonEvent(event.button);
            }
        }
        
        // Small yield to prevent watchdog
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "ButtonDriver.h"

/**
 * @class ButtonHandler
 * @brief BSP Wrapper for Button Driver with press/hold detection on release
 */
class ButtonHandler {
public:
    ButtonHandler(uint8_t pin);
    
    void begin();
    
    /** @brief Call in loop to track press start time */
    void update();

    /** @brief Check for short press (on release) */
    bool checkClicked();
    
    /** @brief Check for long press (on release) */
    bool checkLongPressed();
    
    /** @brief Check current button state */
    bool isPressed();

private:
    ButtonDriver _driver;
};
#endif
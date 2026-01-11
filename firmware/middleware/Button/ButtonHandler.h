#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "ButtonDriver.h"

/**
 * @class ButtonHandler
 * @brief BSP Wrapper for Button Driver
 */
class ButtonHandler {
public:
    /**
     * @brief Constructor
     * @param pin GPIO Pin for the button
     */
    ButtonHandler(uint8_t pin);
    
    void begin();

    bool checkClicked();

private:
    ButtonDriver _driver;
    bool _wasPressed;
};
#endif
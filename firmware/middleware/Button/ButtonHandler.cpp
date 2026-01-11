#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(uint8_t pin)
    : _driver(pin), _wasPressed(false) 
{
}

void ButtonHandler::begin() {
    _driver.init(); // Driver uses init(), not begin()
}

bool ButtonHandler::checkClicked() {
    // Force update driver state (polling)
    _driver.update(); 
    
    // Check state
    bool isPressed = _driver.isPressed();
    
    if (isPressed && !_wasPressed) {
        _wasPressed = true;
        return true; 
    } else if (!isPressed) {
        _wasPressed = false;
    }
    return false;
}
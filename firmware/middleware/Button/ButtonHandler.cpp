#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(uint8_t pin)
    : _driver(pin), _wasPressed(false) 
{
}

void ButtonHandler::begin() {
    _driver.init(); // Init with interrupt
}

bool ButtonHandler::checkClicked() {
    // Driver now uses interrupt, just check flag
    return _driver.checkClicked();
}
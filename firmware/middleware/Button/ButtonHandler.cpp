#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(ButtonDriver* driver) : _driver(driver), _wasPressed(false) {}

bool ButtonHandler::checkClicked() {
    _driver->update();
    bool isCurrentlyPressed = _driver->isPressed();
    bool clicked = false;

    if (!isCurrentlyPressed && _wasPressed) {
        clicked = true;
    }

    _wasPressed = isCurrentlyPressed;
    return clicked;
}
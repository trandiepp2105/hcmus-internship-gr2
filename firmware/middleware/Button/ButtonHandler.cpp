#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(uint8_t pin)
    : _driver(pin)
{
}

void ButtonHandler::begin() {
    _driver.init();
}

void ButtonHandler::update() {
    _driver.updatePressState();
}

bool ButtonHandler::checkClicked() {
    return _driver.checkClicked();
}

bool ButtonHandler::checkLongPressed() {
    return _driver.checkLongPressed();
}

bool ButtonHandler::isPressed() {
    return _driver.isPressed();
}
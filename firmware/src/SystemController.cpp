#include "SystemController.h"

SystemController::SystemController(ButtonHandler* swBtn, LcdHandler* lcdService, PotHandler* potUpper, PotHandler* potLower)
    : _swBtn(swBtn), _lcd(lcdService), _potUpper(potUpper), _potLower(potLower), _currentState(STATE_VALUE) {}

void SystemController::init() {
    _lcd->showStartup();
    updateDisplay(); 
}

void SystemController::update() {
    if (_swBtn->checkClicked()) {
        handleStateTransition();
       
    }
    switch (_currentState) {
        case STATE_SET_UPPER:
            _upperLimit = _potUpper->getScaledValue(0.0f, 14.0f);
            break;

        case STATE_SET_LOWER:
            _lowerLimit = _potLower->getScaledValue(0.0f, 14.0f);
            break;
    }
    updateDisplay();
}

void SystemController::handleStateTransition() {
    if (_currentState == STATE_VALUE) {
        _currentState = STATE_THRESHOLD;
    } else if (_currentState == STATE_THRESHOLD) {
        _currentState = STATE_SET_UPPER;
    } else if (_currentState == STATE_SET_UPPER) {
        _currentState = STATE_SET_LOWER;
    } else if (_currentState == STATE_SET_LOWER) {
        _currentState = STATE_VALUE;
    }
}

void SystemController::updateDisplay() {
    switch (_currentState) {
        case STATE_VALUE:
            _lcd->showValueScreen(_phValue, _tempValue);
            break;
            
        case STATE_THRESHOLD:
        case STATE_SET_UPPER:
        case STATE_SET_LOWER:
            _lcd->showThresholdScreen(_upperLimit, _lowerLimit);
            break;
    }
}
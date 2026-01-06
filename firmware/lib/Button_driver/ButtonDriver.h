#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>

class InputManager {
private:
    uint8_t _pin;
    bool _lastButtonState;      
    bool _currentButtonState;
    unsigned long _lastDebounceTime;
    const unsigned long _debounceDelay = 50; 

public:
    InputManager(uint8_t pin);
    void init();
    void update(); 
    bool isClicked(); 
};

#endif
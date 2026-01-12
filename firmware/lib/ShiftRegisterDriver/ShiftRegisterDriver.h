#ifndef SHIFT_REGISTER_DRIVER_H
#define SHIFT_REGISTER_DRIVER_H

#include <Arduino.h>

/**
 * @class ShiftRegisterDriver
 * @brief Low-level driver for 74HC595 Shift Register IC
 * 
 * Controls 8 outputs using 3 GPIO pins (Data, Clock, Latch)
 */
class ShiftRegisterDriver {
public:
    /**
     * @brief Constructor
     * @param dataPin  DS (Serial Data Input)
     * @param clockPin SHCP (Shift Register Clock)
     * @param latchPin STCP (Storage Register Clock / Latch)
     */
    ShiftRegisterDriver(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);
    
    /**
     * @brief Initialize GPIO pins
     */
    void init();
    
    /**
     * @brief Write 8-bit value to outputs
     * @param data Bitmask for Q0-Q7 (LSB = Q0)
     */
    void write(uint8_t data);
    
    /**
     * @brief Set individual bit
     * @param bit Bit position (0-7)
     * @param state HIGH or LOW
     */
    void setBit(uint8_t bit, bool state);
    
    /**
     * @brief Clear all outputs (all LOW)
     */
    void clearAll();
    
    /**
     * @brief Set all outputs (all HIGH)
     */
    void setAll();
    
    /**
     * @brief Get current output state
     */
    uint8_t getCurrentState();

private:
    uint8_t _dataPin;
    uint8_t _clockPin;
    uint8_t _latchPin;
    uint8_t _currentState;  // Track current output state
    
    void shiftOut();
};

#endif

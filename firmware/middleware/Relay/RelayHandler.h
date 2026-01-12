#ifndef RELAY_HANDLER_H
#define RELAY_HANDLER_H

#include "ShiftRegisterDriver.h"

/**
 * @class RelayHandler
 * @brief BSP/Middleware for controlling 8 relays via 74HC595
 */
class RelayHandler {
public:
    /**
     * @brief Constructor
     * @param dataPin  74HC595 DS pin
     * @param clockPin 74HC595 SHCP pin
     * @param latchPin 74HC595 STCP pin
     */
    RelayHandler(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);
    
    /**
     * @brief Initialize the relay driver
     */
    void begin();
    
    /**
     * @brief Set single relay state
     * @param relayNum Relay number (0-7)
     * @param state true = ON, false = OFF
     */
    void setRelay(uint8_t relayNum, bool state);
    
    /**
     * @brief Set all relays using bitmask
     * @param mask Bitmask (bit 0 = relay 0, etc.)
     */
    void setAllRelays(uint8_t mask);
    
    /**
     * @brief Turn all relays ON
     */
    void allOn();
    
    /**
     * @brief Turn all relays OFF
     */
    void allOff();
    
    /**
     * @brief Get individual relay state
     * @param relayNum Relay number (0-7)
     * @return true if ON, false if OFF
     */
    bool getRelayState(uint8_t relayNum);
    
    /**
     * @brief Get all relay states as bitmask
     */
    uint8_t getCurrentState();

private:
    ShiftRegisterDriver _driver;
};

#endif

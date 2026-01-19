#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include <Arduino.h>

/**
 * @class ButtonDriver
 * @brief Driver nut nhan dung interrupt tren canh len (RISING - khi tha tay).
 * Press = debounce <= holdTime < holdThreshold
 * Hold = holdTime >= holdThreshold
 */
class ButtonDriver {
public:
    ButtonDriver(uint8_t pin);

    /**
     * @brief Khoi tao va gan interrupt (RISING edge)
     */
    void init();

    /**
     * @brief Goi trong loop de cap nhat thoi diem bat dau nhan
     */
    void updatePressState();

    /**
     * @brief Kiem tra neu co short press (debounce <= holdTime < holdThreshold)
     * @return true neu vua co short press
     */
    bool checkClicked();
    
    /**
     * @brief Kiem tra neu co long press (holdTime >= holdThreshold)
     * @return true neu vua co long press
     */
    bool checkLongPressed();
    
    /**
     * @brief Kiem tra trang thai hien tai cua nut
     * @return true neu nut dang duoc nhan
     */
    bool isPressed();

    /**
     * @brief ISR handler (goi boi interrupt tren RISING edge)
     */
    void IRAM_ATTR handleInterrupt();

private:
    uint8_t _pin;
    volatile bool _pressed;           // Flag short press
    volatile bool _longPressed;       // Flag long press
    volatile uint32_t _pressStart;    // Thoi diem bat dau nhan
    volatile uint32_t _lastInterruptTime;
};

#endif
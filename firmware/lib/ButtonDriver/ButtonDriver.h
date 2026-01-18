#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include <Arduino.h>

/**
 * @class ButtonDriver
 * @brief Driver nut nhan dung interrupt de phan hoi nhanh.
 */
class ButtonDriver {
public:
    ButtonDriver(uint8_t pin);

    /**
     * @brief Khoi tao va gan interrupt
     */
    void init();

    /**
     * @brief Kiem tra va tra ve trang thai nhan nut (edge-triggered)
     * @return true neu nut vua duoc nhan (rising edge), false neu khong
     */
    bool checkClicked();

    /**
     * @brief ISR handler (goi boi interrupt)
     */
    void IRAM_ATTR handleInterrupt();

private:
    uint8_t _pin;
    volatile bool _pressed;           // Flag danh dau nut da nhan
    volatile uint32_t _lastInterruptTime;  // Thoi diem interrupt cuoi
    const uint32_t _debounceDelay = 150;   // 150ms debounce (prevent double-click)
};

#endif
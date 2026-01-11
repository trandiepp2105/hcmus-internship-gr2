#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include <Arduino.h>

/**
 * @class ButtonDriver
 * @brief Lop Driver de xu ly tin hieu nut nhan voi logic chong nhieu.
 */
class ButtonDriver {
public:
    /**
     * @brief Khoi tao doi tuong ButtonDriver cho mot chan cu the.
     * @param pin So chan GPIO ma nut nhan duoc ket noi.
     */
    ButtonDriver(uint8_t pin);

    /**
     * @brief Khoi tao
     */
    void init();

    /**
     * @brief Cap nhat trang thai nut nhan va xu ly logic chong nhieu.
     *
     * Ham nay can duoc goi thuong xuyen (vi du: trong vong lap loop)
     * de dam bao trang thai nut nhan duoc cap nhat chinh xac.
     */
    void update();

    /**
     * @brief Kiem tra xem nut nhan hien tai co dang duoc nhan hay khong.
     * @return true neu nut dang nhan, false neu nguoc lai.
     */
    bool isPressed();

private:
    uint8_t _pin;
    bool _currentState;
    bool _lastState;
    uint32_t _lastDebounceTime;
    const uint32_t _debounceDelay = 50; 
};

#endif
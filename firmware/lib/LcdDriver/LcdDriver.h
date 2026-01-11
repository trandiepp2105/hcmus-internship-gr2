#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <LiquidCrystal_I2C.h>

/**
 * @class LcdDriver
 * @brief Lop dieu khien thiet bi LCD qua giao tiep I2C.
 */
class LcdDriver {
public:
    LcdDriver(uint8_t addr, uint8_t cols, uint8_t rows);

    /**
     * @brief Khoi tao man hinh LCD va bat den nen.
     */
    void init();

    /**
     * @brief Xoa toan bo noi dung tren man hinh.
     */
    void clear();

    /**
     * @brief In mot chuoi ky tu tai vi tri chi dinh.
     * @param col Cot bat dau (0-15).
     * @param row Hang bat dau (0-1).
     * @param text Noi dung can in.
     */
    void printAt(uint8_t col, uint8_t row, const String& text);

    /**
     * @brief Bat hoac tat den nen man hinh.
     * @param state true de bat, false de tat.
     */
    void setBacklight(bool state);

private:
    LiquidCrystal_I2C _lcd;
};

#endif
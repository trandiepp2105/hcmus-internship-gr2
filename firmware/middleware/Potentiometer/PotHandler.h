#ifndef POT_HANDLER_H
#define POT_HANDLER_H

#include "PotDriver.h"

class PotHandler {
public:
    PotHandler(uint8_t pin);
    // No begin necessary for Pot usually, but good practice if needed
    
    /**
     * @brief Ánh xạ vị trí của biến trở sang một khoảng giá trị float.
     * @param minVal Giá trị nhỏ nhất (ví dụ: 0.0 cho pH).
     * @param maxVal Giá trị lớn nhất (ví dụ: 14.0 cho pH).
     * @return Giá trị đã được scale nằm trong khoảng từ minVal đến maxVal.
     */
    float getScaledValue(float minVal, float maxVal);

private:
    PotDriver _driver;
    float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
};

#endif
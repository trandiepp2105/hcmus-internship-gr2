#ifndef PH_BSP_H
#define PH_BSP_H

#include <Arduino.h>

/**
 * @class PH_BSP
 * @brief Middleware quản lý riêng cảm biến pH.
 * @note Không chứa logic lưu trữ bên trong.
 */
class PH_BSP {
private:
    uint8_t _pin;
public:
    PH_BSP(uint8_t pin = 34);
    void begin();
    
    /** @brief Chỉ thực hiện nhiệm vụ đọc và trả về giá trị thô */
    float readRaw(); 
};

#endif
#ifndef POT_DRIVER_H
#define POT_DRIVER_H

#include <Arduino.h>

class PotDriver {
public:
    PotDriver(uint8_t pin);
    void init();
    
    /**
     * @brief Đọc và lọc giá trị analog.
     */
    void update();

    /**
     * @brief Lấy giá trị đã lọc ổn định (0-1023).
     */
    uint16_t getValue();

private:
    uint8_t _pin;
    float _filteredValue;
    const float _alpha = 0.5f; // Hệ số làm mượt
};

#endif
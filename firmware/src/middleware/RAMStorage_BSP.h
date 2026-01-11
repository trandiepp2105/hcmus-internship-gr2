#ifndef PH_BSP_H
#define PH_BSP_H

#include <SENSORS.h> // Chỉ giữ lại driver lọc của pH

class PH_BSP {
private:
    SENSORS* _filter;
    uint8_t _pin;
    float _slope;
    float _offset;

public:
    PH_BSP(SENSORS* filterDriver, uint8_t pin = 34);
    void begin();
    float getPH(); // Chỉ trả về giá trị, không lưu trữ trong này
    void setCalibration(float slope, float offset);
};

#endif
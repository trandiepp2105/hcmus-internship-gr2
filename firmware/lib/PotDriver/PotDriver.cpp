#include "PotDriver.h"

PotDriver::PotDriver(uint8_t pin) : _pin(pin), _filteredValue(0.0f) {}

void PotDriver::init() {
    pinMode(_pin, INPUT); 
    _filteredValue = analogRead(_pin); // Lấy giá trị ban đầu làm seed cho bộ lọc
}

void PotDriver::update() {
    int raw = analogRead(_pin);
    // Sử dụng bộ lọc trung bình cộng lũy thừa (EMA)
    _filteredValue += (raw - _filteredValue) * _alpha;
}

uint16_t PotDriver::getValue() {
    return (uint16_t)_filteredValue;
}

/*
Giải thích:
Đoạn code này sử dụng bộ lọc trung bình cộng lũy thừa (EMA)
để làm mượt giá trị đọc từ cảm biến biến trở (potentiometer). 
Bộ lọc EMA giúp giảm nhiễu và dao động đột ngột trong tín hiệu đầu vào, 
giữ lại xu hướng thay đổi thực sự của cảm biến.
*/
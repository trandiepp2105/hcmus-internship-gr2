#include "PH_BSP.h"

/**
 * @brief Khởi tạo lớp PH_BSP với chân Pin chỉ định.
 * @param pin Chân GPIO để đọc tín hiệu Analog (Mặc định là 34).
 */
PH_BSP::PH_BSP(uint8_t pin) {
    this->_pin = pin;
}

/**
 * @brief Cấu hình phần cứng cho cảm biến pH.
 */
void PH_BSP::begin() {
    // Thiết lập chân Pin là đầu vào
    pinMode(_pin, INPUT);
    
    // Cấu hình độ phân giải ADC cho ESP32 là 12-bit (0 - 4095)
    analogReadResolution(12);
}

/**
 * @brief Đọc giá trị pH thô từ cảm biến.
 * @details Hàm này chỉ thực hiện nhiệm vụ đọc ADC và chuyển đổi sang đơn vị pH thô, 
 * không thực hiện lưu trữ hay hiệu chuẩn phức tạp ở đây.
 * @return Giá trị pH thô (float).
 */
float PH_BSP::readRaw() {
    // 1. Đọc giá trị thô từ bộ chuyển đổi ADC
    int rawADC = analogRead(_pin);

    // 2. Chuyển đổi giá trị ADC sang điện áp (Giả sử 0 - 3.3V)
    // Công thức: $V_{out} = \frac{ADC_{raw} \times 3.3}{4095}$
    float voltage = rawADC * (3.3 / 4095.0);

    // 3. Chuyển đổi điện áp sang giá trị pH thô (Dựa trên đặc tính cảm biến)
    // Ví dụ: 0V -> 0 pH, 3.3V -> 14 pH
    float phValue = voltage * (14.0 / 3.3);

    return phValue;
}
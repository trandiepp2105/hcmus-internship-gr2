#ifndef PH_BSP_H
#define PH_BSP_H

#include <STORAGE.h>

/**
 * @brief Định nghĩa chân kết nối cảm biến cho Board hiện tại.
 */
#define PH_SENSOR_PIN 34 // Thay đổi tùy theo board sử dụng

/**
 * @class PH_BSP
 * @brief Board Support Package - Tầng trung gian quản lý cảm biến pH.
 */
class PH_BSP
{
private:
    STORAGE *_storage;

public:
    /**
     * @brief Khởi tạo lớp BSP.
     * @param storageDriver Con trỏ tới Driver lưu trữ sẽ sử dụng.
     */
    PH_BSP(STORAGE *storageDriver);

    /**
     * @brief Đọc giá trị pH thô từ phần cứng.
     * @return Giá trị pH (float) sau khi đã chuyển đổi từ Analog.
     */
    float readPH();

    /**
     * @brief Lưu trữ giá trị pH vào RAM thông qua Driver.
     * @param value Giá trị pH cần lưu.
     */
    void saveData(float value);
};

#endif
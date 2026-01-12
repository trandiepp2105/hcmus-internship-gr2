/**
 * @file TempSensorHandler.h
 * @brief Tầng Middleware/BSP quản lý logic nghiệp vụ cho cảm biến nhiệt độ DS18B20.
 * @details Cung cấp khả năng phân loại lỗi, lọc nhiễu và đóng gói dữ liệu số nguyên.
 */

#ifndef TEMP_SENSOR_HANDLER_H
#define TEMP_SENSOR_HANDLER_H

#include "TempSensorDriver.h"

/**
 * @enum TempError_t
 * @brief Danh sách các mã trạng thái lỗi của cảm biến (Error Status Enum).
 */
enum TempError_t
{
    ERR_NONE = 0,         /**< 0: Hoạt động bình thường */
    ERR_DISCONNECTED = 1, /**< 1: Lỗi mất kết nối vật lý (Mã -127) */
    ERR_CRC_FAIL = 2,     /**< 2: Lỗi nhiễu tín hiệu hoặc sai lệch dữ liệu (Mã -80) */
    ERR_OUT_OF_RANGE = 3, /**< 3: Nhiệt độ vượt quá dải đo (-55°C đến 125°C) */
    ERR_NOT_FOUND = 4     /**< 4: Không tìm thấy hoặc lỗi khởi tạo cảm biến */
};

/**
 * @class TempSensorHandler
 * @brief Lớp điều phối logic đọc và xử lý dữ liệu nhiệt độ.
 */
class TempSensorHandler
{
public:
    /** @brief Hệ số nhân để chuyển float sang integer (giữ 2 chữ số thập phân) */
    static constexpr int SCALE_FACTOR = 100;

    /** @brief Giá trị trả về khi gặp lỗi nặng không thể đọc dữ liệu */
    static constexpr float INVALID_VALUE = -999.0f;

    /**
     * @brief Constructor khởi tạo module.
     * @param pin Chân GPIO kết nối cảm biến (định nghĩa trong bsp_board.h).
     */
    TempSensorHandler(uint8_t pin);

    /** @brief Khởi tạo phần cứng thông qua Driver tầng thấp. */
    void begin();

    /**
     * @brief Lấy giá trị nhiệt độ Celsius đã qua bộ lọc trung bình.
     * @return float Giá trị nhiệt độ thực tế hoặc INVALID_VALUE nếu lỗi.
     */
    float getTemperature();

    /**
     * @brief Chuyển đổi nhiệt độ sang số nguyên (nhân 100) để tối ưu truyền nhận.
     * @return int32_t Giá trị đã scale (Ví dụ: 25.34 -> 2534).
     */
    int32_t getTemperatureScaled();

    /**
     * @brief Lấy mã trạng thái lỗi hiện tại của cảm biến.
     * @return TempError_t Trạng thái từ enum ERR_....
     */
    TempError_t getErrorStatus() const { return _currentError; }

private:
    TempSensorDriver _driver;  /**< Đối tượng Driver thao tác phần cứng */
    TempError_t _currentError; /**< Lưu trữ mã lỗi hiện tại */
    float _lastValidTemp;      /**< Giá trị hợp lệ gần nhất để dự phòng khi nhiễu */

    // Logic bộ lọc Moving Average (10 mẫu) giúp ổn định số đọc
    static const uint8_t FILTER_SIZE = 10;
    float _readings[FILTER_SIZE];
    uint8_t _readIndex;
};

#endif
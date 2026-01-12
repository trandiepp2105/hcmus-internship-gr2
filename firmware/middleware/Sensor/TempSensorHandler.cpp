#include "TempSensorHandler.h"

/**
 * @brief Khởi tạo Handler và nạp giá trị mặc định cho bộ lọc.
 */
TempSensorHandler::TempSensorHandler(uint8_t pin)
    : _driver(pin), _currentError(ERR_NONE), _lastValidTemp(25.0f), _readIndex(0)
{
    // Đổ đầy mảng lọc bằng giá trị an toàn ban đầu
    for (int i = 0; i < FILTER_SIZE; i++)
    {
        _readings[i] = 25.0f;
    }
}

/**
 * @brief Kích hoạt cảm biến Dallas qua OneWire.
 */
void TempSensorHandler::begin()
{
    _driver.init();
}

/**
 * @brief Đọc giá trị từ phần cứng và phân loại 5 trường hợp trạng thái.
 */
float TempSensorHandler::getTemperature()
{
    float raw = _driver.readTemperature();

    // 1. Phân tích và gán mã lỗi (Error Classification)
    if (raw == -127.0f)
    {
        _currentError = ERR_DISCONNECTED; // Lỗi 1: Tuột dây
        return INVALID_VALUE;
    }

    if (raw == -80.0f)
    {
        _currentError = ERR_CRC_FAIL; // Lỗi 2: Nhiễu dữ liệu
        return INVALID_VALUE;
    }

    if (raw < -55.0f || raw > 125.0f)
    {
        _currentError = ERR_OUT_OF_RANGE; // Lỗi 3: Vượt ngưỡng chip
        return INVALID_VALUE;
    }

    if (isnan(raw))
    {
        _currentError = ERR_NOT_FOUND; // Lỗi 4: Không thấy cảm biến
        return INVALID_VALUE;
    }

    // Nếu dữ liệu hợp lệ: Gán mã lỗi 0 (ERR_NONE)
    _currentError = ERR_NONE;

    // 2. Thuật toán lọc trung bình động (Moving Average)
    _readings[_readIndex] = raw;
    _readIndex = (_readIndex + 1) % FILTER_SIZE;

    float sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++)
    {
        sum += _readings[i];
    }

    _lastValidTemp = sum / FILTER_SIZE;
    return _lastValidTemp;
}

/**
 * @brief Thực hiện Scaling dữ liệu sang Integer theo yêu cầu của Lead.
 * @details Công thức: ValueInt = Float * 100. Giúp tiết kiệm RAM và ổn định khi truyền MQTT.
 */
int32_t TempSensorHandler::getTemperatureScaled()
{
    float currentT = getTemperature();

    // Nếu cảm biến đang có lỗi, trả về một mã số nguyên đặc biệt
    if (currentT == INVALID_VALUE)
    {
        return (int32_t)(-999 * SCALE_FACTOR);
    }

    // Chuyển đổi sang số nguyên
    return (int32_t)(currentT * SCALE_FACTOR);
}
#ifndef RAMSTORAGE_H
#define RAMSTORAGE_H

#include <Arduino.h>

/** * @struct TelemetryData
 * @brief Định nghĩa cấu trúc dữ liệu telemetry để lưu trữ khi mất WiFi.
 */
struct TelemetryData {
    float phValue;      // Giá trị pH
    float tempValue;    // Giá trị nhiệt độ
    uint32_t timestamp; // Thời gian ghi nhận (epoch time hoặc uptime)
};

#define MAX_ITEMS 500 // Giới hạn số lượng bản ghi để bảo vệ bộ nhớ DRAM

/**
 * @class RAMStorage
 * @brief Driver quản lý bộ đệm vòng (Circular Buffer) cho dữ liệu Telemetry.
 * @note Đã tách biệt hoàn toàn khỏi logic pH và Calib theo yêu cầu của Lead.
 */
class RAMStorage {
private:
    TelemetryData buffer[MAX_ITEMS]; // Mảng lưu trữ các struct Telemetry
    int head;  // Chỉ số vị trí ghi tiếp theo
    int tail;  // Chỉ số vị trí đọc cũ nhất
    int count; // Số lượng bản ghi hiện có trong bộ nhớ

public:
    /** @brief Khởi tạo các chỉ số điều hướng bộ nhớ */
    RAMStorage();

    /** @brief Đẩy một struct Telemetry vào hàng chờ lưu trữ */
    bool push(TelemetryData data);

    /** @brief Lấy bản ghi Telemetry cũ nhất ra để xử lý */
    bool pop(TelemetryData &output);

    /** @brief Trả về số lượng bản ghi hiện có */
    int getCount();

    /** @brief Kiểm tra bộ nhớ có trống hay không */
    bool isEmpty();
};

#endif
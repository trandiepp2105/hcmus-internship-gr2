#ifndef RAMSTORAGE_BSP_H
#define RAMSTORAGE_BSP_H

#include <RAMStorage.h>

/**
 * @class RAMStorage_BSP
 * @brief Bộ điều phối (Orchestrator) lưu trữ dữ liệu Telemetry.
 */
class RAMStorage_BSP {
private:
    RAMStorage* _driver;

public:
    RAMStorage_BSP(RAMStorage* drv) : _driver(drv) {}

    /** * @brief Đóng gói các giá trị cảm biến vào struct Telemetry và lưu vào RAM.
     * @param ph Giá trị pH đã qua xử lý.
     * @param temp Giá trị nhiệt độ.
     */
    void logTelemetry(float ph, float temp) {
        TelemetryData data;
        data.phValue = ph;
        data.tempValue = temp;
        data.timestamp = millis() / 1000;
        
        _driver->push(data); // Chuyển cho Driver thực hiện lưu trữ thô
    }
};

#endif
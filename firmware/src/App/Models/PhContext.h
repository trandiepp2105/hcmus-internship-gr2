#ifndef PH_CONTEXT_H
#define PH_CONTEXT_H

/**
 * @enum SystemMode
 * @brief Các chế độ hoạt động chính của hệ thống.
 */
enum SystemMode {
    MODE_AUTO,      // Tự động đo và điều khiển
    MODE_MANUAL,    // Đo và giám sát, không tự điều khiển output (hoặc chờ lệnh remote)
    MODE_CONFIG,    // Cấu hình tham số (Threshold, Calibration)
    MODE_INFOR      // Xem thông tin cấu hình
};

/**
 * @enum ConfigState
 * @brief Các trạng thái con trong chế độ CONFIG.
 */
enum ConfigState {
    CFG_THRESHOLD,  // Cấu hình ngưỡng trên/dưới
    CFG_SLOPE,      // Cấu hình Slope (Calib)
    CFG_INTERCEPT   // Cấu hình Intercept (Calib)
};

/**
 * @struct PhContext
 * @brief Cấu trúc dữ liệu trạng thái runtime của hệ thống (RAM).
 */
struct PhContext {
    // Sensor Values
    float currentPh;
    float currentTemp;

    // Outputs Status (True = ON, False = OFF)
    bool output1; // BASE PUMP
    bool output2; // ACID PUMP
    bool output3; // BASE PUMP
    bool output4; // ACID PUMP

    // System State
    SystemMode systemMode;      // Chế độ hiện tại
    ConfigState configState;    // Trạng thái con của Config (nếu đang ở MODE_CONFIG)
    bool isAutoControl;         // True nếu đang cho phép logic tự động can thiệp (redundant with MODE_AUTO but explicitly requested)

    // Constructor defaults
    PhContext() 
        : currentPh(7.0f), 
          currentTemp(25.0f), 
          output1(false), output2(false), output3(false), output4(false),
          systemMode(MODE_AUTO),
          configState(CFG_THRESHOLD),
          isAutoControl(true) {}
};

#endif // PH_CONTEXT_H

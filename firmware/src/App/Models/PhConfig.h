#ifndef PH_CONFIG_H
#define PH_CONFIG_H

/**
 * @struct PhConfig
 * @brief Cấu trúc dữ liệu lưu trữ cấu hình hệ thống (lưu trong Flash/EEPROM).
 */
struct PhConfig {
    float phUpperLimit;     // Ngưỡng pH trên (ví dụ: > 8.5 thì bơm Acid)
    float phLowerLimit;     // Ngưỡng pH dưới (ví dụ: < 6.5 thì bơm Base)
    float calibSlope;       // Hệ số góc đường chuẩn (Calibration Slope)
    float calibIntercept;   // Hệ số chặn đường chuẩn (Calibration Intercept)

    // Constructor with default values
    PhConfig() 
        : phUpperLimit(8.5f), 
          phLowerLimit(6.5f), 
          calibSlope(1.0f), 
          calibIntercept(0.0f) {}
};

#endif // PH_CONFIG_H

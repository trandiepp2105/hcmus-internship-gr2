#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include "LcdDriver.h"

/**
 * @class DisplayService
 * @brief Lop dich vu quan ly cac kieu hien thi khac nhau tren man hinh.
 */
class LcdHandler {
public:
    LcdHandler(LcdDriver* driver);

    /**
     * @brief Hien thi man hinh chao mung khi khoi dong.
     */
    void showStartup();

    /**
     * @brief Hien thi man hinh do gia tri pH va Nhiet do.
     */
    void showValueScreen(float ph, float temp);

    /**
     * @brief Hien thi man hinh cau hinh nguong Upper va Lower.
     */
    void showThresholdScreen(float upper, float lower);

    /**
     * @brief Hien thi thong bao loi he thong.
     */
    void showError(const String& msg);

    /**
     * @brief Cap nhat trang thai ket noi WiFi.
     */
    void showWiFiStatus(const String& status);

private:
    LcdDriver* _lcd;
};

#endif
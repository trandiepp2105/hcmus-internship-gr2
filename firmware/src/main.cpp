#include <Arduino.h>
#include <RAMStorage.h>
#include "middleware/PH_BSP.h"
#include "middleware/RAMStorage_BSP.h"

// 1. Khởi tạo các đối tượng Driver (Thấp nhất)
RAMStorage ramDriver;

// 2. Khởi tạo các bộ BSP riêng biệt (Tầng giữa)
PH_BSP phSys(34);
RAMStorage_BSP storageSys(&ramDriver);

void setup() {
    Serial.begin(115200);
    phSys.begin();
    Serial.println("System Initialized: Decoupled Architecture with Struct Telemetry");
}

void loop() {
    // BƯỚC 1: Đọc dữ liệu từ bộ BSP pH độc lập
    float currentPH = phSys.readRaw();
    float currentTemp = 25.5; // Giả lập giá trị nhiệt độ

    // BƯỚC 2: Chuyển dữ liệu sang bộ BSP lưu trữ để đóng gói vào struct
    storageSys.logTelemetry(currentPH, currentTemp);

    // Kiểm tra dữ liệu
    Serial.printf("Logged -> pH: %.2f, Records in RAM: %d\n", currentPH, ramDriver.getCount());
    
    delay(3000);
}
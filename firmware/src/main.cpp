#include "middleware/PH_BSP.h"
#include "middleware/STORAGE_BSP.h"
#include <SENSORS.h>
#include <STORAGE.h>

SENSORS phFilter;
STORAGE storageDriver;

PH_BSP phSys(&phFilter);
STORAGE_BSP storageSys(&storageDriver);

void loop() {
    float currentPH = phSys.getPH();      // Bước 1: Lấy dữ liệu từ pH
    storageSys.saveData(currentPH);      // Bước 2: Đưa sang Storage để lưu
    delay(2000);
}
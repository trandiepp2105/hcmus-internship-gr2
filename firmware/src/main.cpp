#include <STORAGE.h>
#include "middleware/PH_BSP.h"

STORAGE ramDriver;
PH_BSP phController(&ramDriver);

void setup()
{
  Serial.begin(115200);
  Serial.println("System Layered Architecture Initialized.");
}

void loop()
{
  // 1. Đọc dữ liệu từ BSP
  float currentPH = phController.readPH();

  // 2. Yêu cầu BSP lưu dữ liệu (BSP sẽ tự gọi Driver)
  phController.saveData(currentPH);

  Serial.printf("Current pH: %.2f | Records in RAM: %d\n",
                currentPH, ramDriver.getCount());

  delay(3000);
}
#ifndef PH_CONTROLLER_H
#define PH_CONTROLLER_H

#include <Arduino.h>
#include "../Models/PhConfig.h"
#include "../Models/PhContext.h"

// Dependencies (Assuming Headers exist)
#include "../../../lib/Storage/Storage.h"
// #include "../../../lib/BSP/IOExpander/IOExpanderBSP.h"
#include "../../middleware/Button/ButtonHandler.h"
#include "../../middleware/Tft/TftHandler.h"
#include "../../middleware/Wifi/WifiHandler.h"
#include "../../middleware/Potentiometer/PotHandler.h"
#include "../../middleware/Sensor/TempSensorHandler.h"
#include "../../middleware/Relay/RelayHandler.h"
#include "../../middleware/Mqtt/MqttHandler.h"

// ======== Configuration ========
#define PH_SAMPLE_INTERVAL_MS 5000  // Read sensors every 5 seconds

// Network Config
#define WIFI_AP_NAME          "PH CONTROLLER SETUP"
#define MQTT_SERVER           "192.168.1.16"  // Change to your server
#define MQTT_PORT             1883
#define TB_DEVICE_NAME        "PH_CONTROLLER_006"
#define TB_PROVISION_KEY      "7b489653-2d36-45bd-9476-c7aa5b9ae5fc"    // Set your key
#define TB_PROVISION_SECRET   "5BnZwv6WnuNbs6E45yNq" // Set your secret

/**
 * @class PhController
 * @brief Lớp điều khiển chính của ứng dụng pH Controller.
 * Quản lý trạng thái hệ thống, đọc cảm biến, xử lý logic và điều khiển ngõ ra.
 */
class PhController {
public:
    /**
     * @brief Constructor
     * @param storage Driver lưu trữ cấu hình
     * @param ioExpander Driver IO mở rộng (Relays, LEDs)
     * @param btnA Nút chuyển chế độ
     * @param btnB Nút điều hướng/chọn
     * @param lcd Màn hình hiển thị
     * @param potUpper Biến trở chỉnh ngưỡng trên
     * @param potLower Biến trở chỉnh ngưỡng dưới
     */
    PhController(Storage* storage, 
                 // IOExpanderBSP* ioExpander,
                 ButtonHandler* btnA, 
                 ButtonHandler* btnB,
                 TftHandler* tft,
                 WifiHandler* wifi,
                 PotHandler* potUpper,
                 PotHandler* potLower,
                 TempSensorHandler* tempSensor,
                 RelayHandler* relayHandler,
                MqttHandler* mqttHandler);

    /**
     * @brief Khởi tạo hệ thống (Load config, Init hardware)
     */
    void begin();

    /**
     * @brief Vòng lặp chính, gọi liên tục trong loop()
     */
    void update();

    /**
     * @brief Test LCD display with incrementing values
     */
    void testLcd();
    
    /**
     * @brief Handle button inputs (should be called first in loop for responsiveness)
     */
    void handleInputs();
    
    /**
     * @brief Set control mode from MQTT callback
     */
    void setControlMode(bool isAuto);
    
    /**
     * @brief Handle RPC setRelay command
     * @return true if command was accepted (MANUAL mode), false if rejected (AUTO mode)
     */
    bool handleRpcSetRelay(int relay, bool state);
    
    /**
     * @brief Sync control mode to ThingsBoard (call after MQTT connects)
     */
    void syncControlMode();
    
    /**
     * @brief Sync thresholds to ThingsBoard (call after MQTT connects)
     */
    void syncThresholds();
    
    /**
     * @brief Handle threshold update from ThingsBoard
     */
    void handleThresholdUpdate(float minThreshold, float maxThreshold);

private:
    // Dependencies
    Storage* _storage;
    // IOExpanderBSP* _ioExpander;
    ButtonHandler* _btnA;
    ButtonHandler* _btnB;
    TftHandler* _tft;
    WifiHandler* _wifi;
    PotHandler* _potUpper;
    PotHandler* _potLower;
    TempSensorHandler* _tempSensor;
    RelayHandler* _relayHandler;
    MqttHandler* _mqttHandler;

    // Data Models
    PhConfig _config;
    PhContext _context;
    PhContext _lastContext; // To track changes for optimization
    unsigned long _lastSampleTime = 0; // For periodic sampling
    bool _forceDisplayUpdate = true; // Force first update

    // --- Internal Logic Methods ---

    /**
     * @brief Đọc cảm biến (pH, Temp, Pots)
     */
    void readSensors();

    /**
     * @brief Logic điều khiển cho chế độ AUTO
     */
    void runAutoLogic();

    /**
     * @brief Logic điều khiển cho chế độ MANUAL
     */
    void runManualLogic();

    /**
     * @brief Logic điều khiển cho chế độ CONFIG
     */
    void runConfigLogic();

    /**
     * @brief Logic điều khiển cho chế độ INFOR
     */
    void runInforLogic();

    /**
     * @brief Cập nhật hiển thị LCD dựa trên Context
     */
    void updateDisplay();

    /**
     * @brief Cập nhật các Output (Pumps, Mixer...) ra phần cứng
     */
    void updateOutputs();

    /**
     * @brief Tắt an toàn tất cả thiết bị điều khiển (Pumps, Mixer)
     */
    void stopAllActuators();

    /**
     * @brief Lưu cấu hình hiện tại xuống Flash
     */
    void saveConfig();

    /**
     * @brief Load cấu hình từ Flash
     */
    void loadConfig();
};

#endif // PH_CONTROLLER_H

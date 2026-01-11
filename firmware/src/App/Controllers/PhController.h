#ifndef PH_CONTROLLER_H
#define PH_CONTROLLER_H

#include <Arduino.h>
#include "../Models/PhConfig.h"
#include "../Models/PhContext.h"

// Dependencies (Assuming Headers exist)
#include "../../../lib/Storage/Storage.h"
// #include "../../../lib/BSP/IOExpander/IOExpanderBSP.h"
#include "../../middleware/Button/ButtonHandler.h"
#include "../../middleware/Lcd/LcdHandler.h"
#include "../../middleware/Potentiometer/PotHandler.h"

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
                 LcdHandler* lcd,
                 PotHandler* potUpper,
                 PotHandler* potLower);

    /**
     * @brief Khởi tạo hệ thống (Load config, Init hardware)
     */
    void begin();

    /**
     * @brief Vòng lặp chính, gọi liên tục trong loop()
     */
    void update();

private:
    // Dependencies
    Storage* _storage;
    // IOExpanderBSP* _ioExpander;
    ButtonHandler* _btnA;
    ButtonHandler* _btnB;
    LcdHandler* _lcd;
    PotHandler* _potUpper;
    PotHandler* _potLower;

    // Data Models
    PhConfig _config;
    PhContext _context;

    // --- Internal Logic Methods ---

    /**
     * @brief Đọc cảm biến (pH, Temp, Pots)
     */
    void readSensors();

    /**
     * @brief Xử lý sự kiện nút bấm (State Transitions)
     */
    void handleInputs();

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

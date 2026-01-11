#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H

#include "../middleware/Button/ButtonHandler.h"
#include "../middleware/Lcd/LcdHandler.h"
#include "../middleware/Potentiometer/PotHandler.h"

/**
 * @class SystemController
 * @brief Lop dieu khien chinh (Application Layer) quan ly logic toan he thong.
 */
class SystemController {
public:
    enum State {
        STATE_VALUE,      // Man hinh hien thi pH va Temp
        STATE_THRESHOLD,   // Man hinh hien thi nguong Upper/Lower
        STATE_SET_UPPER,   // Man hinh hien thi nguong Upper
        STATE_SET_LOWER    // Man hinh hien thi nguong Lower
    };

    SystemController(ButtonHandler* swBtn, LcdHandler* lcdService, PotHandler* potUpper, PotHandler* potLower);

    /**
     * @brief Ham khoi tao ban dau cho controller.
     */
    void init();

    /**
     * @brief Ham cap nhat logic, can duoc goi lien tuc trong loop().
     */
    void update();

private:
    ButtonHandler* _swBtn;
    LcdHandler* _lcd;
    PotHandler* _potUpper;
    PotHandler* _potLower;

    State _currentState;


    //data for testing
    float _phValue = 7.05;
    float _tempValue = 25.5;
    float _upperLimit = 8.5;
    float _lowerLimit = 6.5;

    void handleStateTransition();
    void updateDisplay();
};

#endif
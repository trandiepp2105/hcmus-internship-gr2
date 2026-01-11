#include "PhController.h"
#include "../../middleware/bsp_board.h" // For pin defs if needed, though passed via Dependency Injection

PhController::PhController(Storage* storage, 
                           // IOExpanderBSP* ioExpander,
                           ButtonHandler* btnA, 
                           ButtonHandler* btnB,
                           LcdHandler* lcd,
                           PotHandler* potUpper,
                           PotHandler* potLower)
    : _storage(storage), 
      // _ioExpander(ioExpander), 
      _btnA(btnA), 
      _btnB(btnB), 
      _lcd(lcd), 
      _potUpper(potUpper), 
      _potLower(potLower) {
}

void PhController::begin() {
    // 1. Load Config
    loadConfig();

    // 2. Init Hardware (Drivers usually init outside or here if specific logic needed)
    // Assuming Drivers are .begin() call in main or SystemController init
    
    // 3. Set Initial State
    _context.systemMode = MODE_AUTO;
    _context.isAutoControl = true;
    
    Serial.println("[PhController] Started.");
}

void PhController::update() {
    // 1. Read Inputs
    readSensors();
    handleInputs();

    // 2. State Machine Logic
    switch (_context.systemMode) {
        case MODE_AUTO:
            runAutoLogic();
            break;
        case MODE_MANUAL:
            runManualLogic();
            break;
        case MODE_CONFIG:
            runConfigLogic();
            break;
        case MODE_INFOR:
            runInforLogic();
            break;
    }

    // 3. Update Hardware
    updateOutputs();  // Relays
    updateDisplay();  // LCD
}

void PhController::readSensors() {
    // Read Temperature (Placeholder)
    _context.currentTemp = 25.0f; // Replace with actual sensor reading

    // Read pH (Placeholder)
    // float rawAdc = analogRead(...);
    // _context.currentPh = convertToPh(rawAdc, _config.calibSlope, _config.calibIntercept);
    _context.currentPh = 7.05f; // Fake value for now
}

void PhController::handleInputs() {
    // --- Button A: Mode Switching ---
    if (_btnA->checkClicked()) { // Assuming isPressed handles debounce and returns true once on press
        Serial.println("[Input] Button A Pressed -> Changing Mode");
        switch (_context.systemMode) {
            case MODE_AUTO:
                _context.systemMode = MODE_MANUAL;
                _context.isAutoControl = false;
                break;
            case MODE_MANUAL:
                _context.systemMode = MODE_CONFIG;
                _context.isAutoControl = false;
                stopAllActuators(); // Safety first
                break;
            case MODE_CONFIG:
                _context.systemMode = MODE_INFOR;
                _context.isAutoControl = false;
                stopAllActuators();
                saveConfig(); // Auto-save on exit config? Or explicit save? Assuming auto-save for now.
                break;
            case MODE_INFOR:
                _context.systemMode = MODE_AUTO;
                _context.isAutoControl = true;
                break;
        }
    }

    // --- Button B: Action/Select ---
    if (_btnB->checkClicked()) {
        Serial.println("[Input] Button B Pressed");
        if (_context.systemMode == MODE_CONFIG) {
            // Cycle Config Pages
            switch (_context.configState) {
                case CFG_THRESHOLD: _context.configState = CFG_SLOPE; break;
                case CFG_SLOPE:     _context.configState = CFG_INTERCEPT; break;
                case CFG_INTERCEPT: _context.configState = CFG_THRESHOLD; break;
            }
        }
    }
}

void PhController::runAutoLogic() {
    // Simple Hysteresis Control
    if (_context.currentPh > _config.phUpperLimit) {
        _context.output1 = true;  // Pump Acid ON
        _context.output2 = false; // Pump Base OFF
        _context.output4 = true;  // Alarm ON?
    } else if (_context.currentPh < _config.phLowerLimit) {
        _context.output1 = false;
        _context.output2 = true;  // Pump Base ON
        _context.output4 = true;
    } else {
        _context.output1 = false;
        _context.output2 = false;
        _context.output4 = false; // All OK
    }
    
    // Mixer Logic (Always ON in Auto? Or Periodic? Assuming OFF for basic test or specific logic needed)
    _context.output3 = false; 
}

void PhController::runManualLogic() {
    // Do nothing logic-wise, outputs stay as last set OR controlled via Remote (ThingsBoard) implementation later
    // For safety, ensure we don't accidentally toggle pumps here.
}

void PhController::runConfigLogic() {
    // In Config Mode, we read Potentiometers and update Config IMMEDIATELY (Live Preview)
    if (_context.configState == CFG_THRESHOLD) {
        // Map Potentiometer (0-4095 or 0-100%) to pH Range (0-14)
        // Using PotHandler to get percentage or raw? Assuming getPercentage() returns 0.0-1.0
         float valUpper = _potUpper->getScaledValue(0, 100); // 0-100
         float valLower = _potLower->getScaledValue(0, 100); // 0-100
         
         // Mapping 0-100 -> 0-14pH (Example)
         _config.phUpperLimit = (valUpper / 100.0f) * 14.0f;
         _config.phLowerLimit = (valLower / 100.0f) * 14.0f;
    }
}

void PhController::runInforLogic() {
    // Read-only, logic does nothing
}

void PhController::updateDisplay() {
    // Delegate to LCD Handler based on Mode - Placeholder for now
    // _lcd->showStatus(_context);
}

void PhController::updateOutputs() {
    // if (_ioExpander) {
    //     uint8_t portVal = 0;
    //     if (_context.output1) portVal |= (1 << 0);
    //     if (_context.output2) portVal |= (1 << 1);
    //     if (_context.output3) portVal |= (1 << 2);
    //     if (_context.output4) portVal |= (1 << 3);
        
    //     _ioExpander->setPortValue(portVal);
    // }
    // Temporary: Print output state to Serial for debugging
    if (_context.output1) Serial.print(" [ACID ON]");
    if (_context.output2) Serial.print(" [BASE ON]");
    // ... etc
}

void PhController::stopAllActuators() {
    _context.output1 = false;
    _context.output2 = false;
    _context.output3 = false;
    _context.output4 = false;
    updateOutputs(); // Apply immediately
}

void PhController::loadConfig() {
    // Load from Storage
    _config.phUpperLimit = _storage->getDouble("ph_upper", 8.5);
    _config.phLowerLimit = _storage->getDouble("ph_lower", 6.5);
    _config.calibSlope   = _storage->getDouble("calib_slope", 1.0);
    _config.calibIntercept = _storage->getDouble("calib_int", 0.0);
}

void PhController::saveConfig() {
    // Save to Storage
    _storage->putDouble("ph_upper", _config.phUpperLimit);
    _storage->putDouble("ph_lower", _config.phLowerLimit);
    _storage->putDouble("calib_slope", _config.calibSlope);
    _storage->putDouble("calib_int", _config.calibIntercept);
    Serial.println("[Config] Saved.");
}

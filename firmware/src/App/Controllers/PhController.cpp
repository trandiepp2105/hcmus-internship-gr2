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
    
    // 4. Update Tracking State (Must be last)
    _lastContext = _context;
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
    // Control Logic:
    // pH > Upper: Output 2 (ACID) & 4 (ACID) -> ON
    // pH < Lower: Output 1 (BASE) & 3 (BASE) -> ON
    // Lower <= pH <= Upper: ALL OFF
    
    if (_context.currentPh > _config.phUpperLimit) {
        _context.output1 = false; // Base OFF
        _context.output2 = true;  // Acid ON
        _context.output3 = false; // Base OFF
        _context.output4 = true;  // Acid ON
    } else if (_context.currentPh < _config.phLowerLimit) {
        _context.output1 = true;  // Base ON
        _context.output2 = false; // Acid OFF
        _context.output3 = true;  // Base ON
        _context.output4 = false; // Acid OFF
    } else {
        _context.output1 = false;
        _context.output2 = false;
        _context.output3 = false;
        _context.output4 = false; // All OK
    }
}

void PhController::runManualLogic() {
    // Do nothing logic-wise, outputs stay as last set OR controlled via Remote (ThingsBoard) implementation later
    // For safety, ensure we don't accidentally toggle pumps here.
}

void PhController::runConfigLogic() {
    // In Config Mode, we read Potentiometers and update Config IMMEDIATELY (Live Preview)
    if (_context.configState == CFG_THRESHOLD) {
         float valUpper = _potUpper->getScaledValue(0, 100); // 0-100
         float valLower = _potLower->getScaledValue(0, 100); // 0-100
         
         _config.phUpperLimit = (valUpper / 100.0f) * 14.0f;
         _config.phLowerLimit = (valLower / 100.0f) * 14.0f;
    }
}

void PhController::runInforLogic() {
    // Read-only, logic does nothing
}

void PhController::updateDisplay() {
    // Only update if something relevant changed (Simple Logic)
    // For now, we update if Mode changes or periodically (or relying on LcdHandler optimization if implemented, but here we control call)
    // Current primitive change detection:
    bool modeChanged = (_context.systemMode != _lastContext.systemMode);
    bool configStateChanged = (_context.configState != _lastContext.configState);
    bool valueChanged = abs(_context.currentPh - _lastContext.currentPh) > 0.05 ||
                        abs(_context.currentTemp - _lastContext.currentTemp) > 0.5;
    bool thresholdChanged = abs(_config.phUpperLimit - _lastContext.currentPh) > 0.05 || // Warning: comparing Config vs LastPh? Mistake in logic previously
                            abs(_config.phUpperLimit - _lastContext.output1 ) > 999; // Dummy Check
                            
    // Better Logic:
    // 1. Clear screen on Mode Change
    if (modeChanged) {
        // _lcd->clear(); // LcdHandler might need a clear method exposed or handle it inside Show methods
         // Currently LcdHandler doesn't expose clear directly except via wrapper? 
         // Looking at LcdHandler.cpp, it calls _lcd->printAt. 
         // It implies we just overwrite.
    }

    if (modeChanged || valueChanged || configStateChanged) {
        switch (_context.systemMode) {
            case MODE_AUTO:
                _lcd->showValueScreen(_context.currentPh, _context.currentTemp);
                break;
            case MODE_MANUAL:
                // Show pH and Temp
                // Note: Manual mode might need "MANUAL" text. Current Handler showValueScreen only shows pH/Temp.
                // We might need to extend LcdHandler later. For now use what we have.
                // If Manual, maybe we want to indicate it?
                // The current LcdHandler::showValueScreen just prints "pH: ... Temp: ...".
                _lcd->showValueScreen(_context.currentPh, _context.currentTemp);
                break;
                
            case MODE_CONFIG:
                if (_context.configState == CFG_THRESHOLD) {
                    _lcd->showThresholdScreen(_config.phUpperLimit, _config.phLowerLimit);
                } else {
                    // Placeholder for Slope/Intercept screens if LcdHandler supported them
                    // _lcd->showConfigScreen(...);
                    // Retain last screen or show "Config: Slope" manually?
                    // Let's rely on Threshold screen for now or add methods.
                }
                break;
                
            case MODE_INFOR:
                 // Re-use Threshold Screen for Information?
                 _lcd->showThresholdScreen(_config.phUpperLimit, _config.phLowerLimit);
                 break;
        }
    }
}

void PhController::updateOutputs() {
    // Check if any output state changed
    bool changed = (_context.output1 != _lastContext.output1) ||
                   (_context.output2 != _lastContext.output2) ||
                   (_context.output3 != _lastContext.output3) ||
                   (_context.output4 != _lastContext.output4);

    if (changed) {
        // if (_ioExpander) { ... }
        
        Serial.print("[Output] State Changed: ");
        if (_context.output1) Serial.print("BASE(1)=ON "); else Serial.print("BASE(1)=OFF ");
        if (_context.output2) Serial.print("ACID(2)=ON "); else Serial.print("ACID(2)=OFF ");
        if (_context.output3) Serial.print("BASE(3)=ON "); else Serial.print("BASE(3)=OFF ");
        if (_context.output4) Serial.print("ACID(4)=ON"); else Serial.print("ACID(4)=OFF");
        Serial.println();
    }
    
    // Note: Tracking update moved to main update() loop
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

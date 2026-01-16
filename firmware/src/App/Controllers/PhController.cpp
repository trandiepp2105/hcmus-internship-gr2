#include "PhController.h"
#include "../../middleware/bsp_board.h" // For pin defs if needed, though passed via Dependency Injection

PhController::PhController(Storage* storage, 
                           // IOExpanderBSP* ioExpander,
                           ButtonHandler* btnA, 
                           ButtonHandler* btnB,
                           LcdHandler* lcd,
                           PotHandler* potUpper,
                           PotHandler* potLower,
                           TempSensorHandler* tempSensor,
                           RelayHandler* relayHandler,
                           MqttHandler* mqttHandler)
    : _storage(storage), 
      // _ioExpander(ioExpander), 
      _btnA(btnA), 
      _btnB(btnB), 
      _lcd(lcd), 
      _potUpper(potUpper), 
      _potLower(potLower),
      _tempSensor(tempSensor),
      _relayHandler(relayHandler),
      _mqttHandler(mqttHandler) {
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
    // 1. Handle Button Inputs (Always responsive)
    handleInputs();

    // 2. Mode-specific Logic
    switch (_context.systemMode) {
        case MODE_AUTO:
            // Periodic Tasks - Every 5 Seconds
            if (millis() - _lastSampleTime >= PH_SAMPLE_INTERVAL_MS) {
                _lastSampleTime = millis();
                readSensors();

                if (_mqttHandler && _mqttHandler->isConnected()) {
                    uint8_t relayMask = 0;
                    if (_context.output1) relayMask |= 0x01;
                    if (_context.output2) relayMask |= 0x02;
                    if (_context.output3) relayMask |= 0x04;
                    if (_context.output4) relayMask |= 0x08;

                    // Sending data to Middleware
                    _mqttHandler->pushTelemetry(
                        _context.currentPh, 
                        _context.currentTemp, 
                        relayMask, 
                        _context.systemMode, 
                        0 // errorCode
                    );
                // Log context
                Serial.printf("[Context] pH=%.2f | Temp=%.1f | Mode=AUTO | Out=%d%d%d%d\n",
                    _context.currentPh, _context.currentTemp,
                    _context.output1, _context.output2, _context.output3, _context.output4);
                
                runAutoLogic();
                updateOutputs();
            }
            break;
        }
        case MODE_MANUAL:
            // Periodic Tasks - Every 5 Seconds
            if (millis() - _lastSampleTime >= PH_SAMPLE_INTERVAL_MS) {
                _lastSampleTime = millis();
                readSensors();
                
                // Log context
                Serial.printf("[Context] pH=%.2f | Temp=%.1f | Mode=MANUAL | Out=%d%d%d%d\n",
                    _context.currentPh, _context.currentTemp,
                    _context.output1, _context.output2, _context.output3, _context.output4);
                
                runManualLogic();
                updateOutputs();
            }
            break;
            
        case MODE_CONFIG:
            // Config Mode: Continuous potentiometer reading (no 5s delay)
            runConfigLogic();
            break;
            
        case MODE_INFOR:
            // Info Mode: Just display stored config (no sensor reading needed)
            runInforLogic();
            break;
    }

    // 3. Update Display (Always - handles its own change detection)
    updateDisplay();  
    
    // 4. Update Tracking State (Must be last)
    _lastContext = _context;
}

void PhController::readSensors() {
    // Read Temperature (Real)
    float t = _tempSensor->getTemperature();
    // DS18B20 returns -127.0 if error (disconnected)
    if (t > -100.0) {
        _context.currentTemp = t;
    } else {
        // Keep last valid or set error flag? For now keep last valid or default 25
        // _context.currentTemp = 25.0f; // Optional fallback
    }

    // Read pH (Placeholder)
    // float rawAdc = analogRead(...);
    // _context.currentPh = convertToPh(rawAdc, _config.calibSlope, _config.calibIntercept);
    //_context.currentPh = 7.05f; // Fake value for now
    //=====// ĐOẠN TEST GIÁ TRỊ pH RANDOM 4 MỨC //=====//
    static int testStep = 0;
    float testPhValues[4];
    testPhValues[0] = _config.phLowerLimit - 0.5f; // Dưới ngưỡng lower
    testPhValues[1] = _config.phLowerLimit + 0.5f; // Trong ngưỡng (gần lower)
    testPhValues[2] = _config.phUpperLimit - 0.5f; // Trong ngưỡng (gần upper)
    testPhValues[3] = _config.phUpperLimit + 0.5f; // Trên ngưỡng upper

    _context.currentPh = testPhValues[testStep];
    testStep = (testStep + 1) % 4;
    //Serial.printf("//=====// TEST pH: %.2f //=====//\n", _context.currentPh);
    //=====// KẾT THÚC ĐOẠN TEST //=====//

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
    // In Config Mode, we read Potentiometers and update Config only at X.0/X.5 milestones
    if (_context.configState == CFG_THRESHOLD) {
         float valUpper = _potUpper->getScaledValue(0, 100); // 0-100
         float valLower = _potLower->getScaledValue(0, 100); // 0-100
         
         // Hysteresis: Only process if raw value changed significantly (reduce noise)
         static float lastRawUpper = -100.0;
         static float lastRawLower = -100.0;
         const float HYSTERESIS = 2.0; // Deadzone ~2% of pot range
         
         bool rawChanged = (abs(valUpper - lastRawUpper) > HYSTERESIS) ||
                          (abs(valLower - lastRawLower) > HYSTERESIS);
         
         if (!rawChanged) return; // Skip if noise only
         
         lastRawUpper = valUpper;
         lastRawLower = valLower;
         
         // Map to pH range 0-14
         float rawUpper = (valUpper / 100.0f) * 14.0f;
         float rawLower = (valLower / 100.0f) * 14.0f;
         
         // Round to 2 decimal places
         float roundedUpper = round(rawUpper * 100.0f) / 100.0f;
         float roundedLower = round(rawLower * 100.0f) / 100.0f;
         
         // Check if value is exactly X.0 or X.5 (tolerance 0.01)
         float fracUpper = fmod(roundedUpper, 0.5f);
         float fracLower = fmod(roundedLower, 0.5f);
         bool upperIsMilestone = (fracUpper < 0.02f) || (fracUpper > 0.48f);
         bool lowerIsMilestone = (fracLower < 0.02f) || (fracLower > 0.48f);
         
         if (!upperIsMilestone && !lowerIsMilestone) return; // Not at milestone yet
         
         // Snap to exact X.0 or X.5 if at milestone
         float snappedUpper = upperIsMilestone ? (round(roundedUpper * 2.0f) / 2.0f) : _config.phUpperLimit;
         float snappedLower = lowerIsMilestone ? (round(roundedLower * 2.0f) / 2.0f) : _config.phLowerLimit;
         
         // Only update and log if value changed
         static float lastUp = -1.0;
         static float lastLow = -1.0;
         
         if (snappedUpper != lastUp || snappedLower != lastLow) {
             _config.phUpperLimit = snappedUpper;
             _config.phLowerLimit = snappedLower;
             Serial.printf("[Config] Upper: %.1f | Lower: %.1f\n", snappedUpper, snappedLower);
             lastUp = snappedUpper;
             lastLow = snappedLower;
         }
    }
    else if (_context.configState == CFG_SLOPE) {
        Serial.println("[Config] Slope configuration via pot not implemented yet.");
    }
    else if (_context.configState == CFG_INTERCEPT) {
        Serial.println("[Config] Intercept configuration via pot not implemented yet.");
    }
}

void PhController::runInforLogic() {
    // Logic for Info Mode
    // The LCD update happens in updateDisplay().
    // We can add a periodic log here to confirm values.
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 2000) {
        Serial.printf("[Info] Config in Memory: Upper=%.2f, Lower=%.2f\n", 
                      _config.phUpperLimit, _config.phLowerLimit);
        lastLog = millis();
    }
}

void PhController::updateDisplay() {
    // Check if any relevant value changed
    bool modeChanged = (_context.systemMode != _lastContext.systemMode);
    bool configStateChanged = (_context.configState != _lastContext.configState);
    bool valueChanged = (abs(_context.currentPh - _lastContext.currentPh) > 0.01) ||
                        (abs(_context.currentTemp - _lastContext.currentTemp) > 0.3);
    
    // Force update on first call, mode change, or value change
    bool needUpdate = _forceDisplayUpdate || modeChanged || configStateChanged || valueChanged;
    
    if (needUpdate) {
        _forceDisplayUpdate = false; // Clear force flag
        
        switch (_context.systemMode) {
            case MODE_AUTO:
            case MODE_MANUAL:
                _lcd->showValueScreen(_context.currentPh, _context.currentTemp);
                break;
                
            case MODE_CONFIG:
                _lcd->showThresholdScreen(_config.phUpperLimit, _config.phLowerLimit);
                break;
                
            case MODE_INFOR:
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
        // Build bitmask for 8-relay module (only use bits 0-3, bits 4-7 = 0)
        uint8_t relayMask = 0;
        if (_context.output1) relayMask |= 0x01;  // Bit 0 = Relay 1 (BASE)
        if (_context.output2) relayMask |= 0x02;  // Bit 1 = Relay 2 (ACID)
        if (_context.output3) relayMask |= 0x04;  // Bit 2 = Relay 3 (BASE)
        if (_context.output4) relayMask |= 0x08;  // Bit 3 = Relay 4 (ACID)
        // Bits 4-7 remain 0
        
        // Drive actual relays
        if (_relayHandler) {
            _relayHandler->setAllRelays(relayMask);
        }
        
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

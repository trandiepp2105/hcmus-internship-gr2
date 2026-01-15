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
                           RelayHandler* relayHandler)
    : _storage(storage), 
      // _ioExpander(ioExpander), 
      _btnA(btnA), 
      _btnB(btnB), 
      _lcd(lcd), 
      _potUpper(potUpper), 
      _potLower(potLower),
      _tempSensor(tempSensor),
      _relayHandler(relayHandler) {
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

void PhController::testLcd() {
    static int counter = 0;
    static unsigned long lastUpdate = 0;
    
    // Update every 2 seconds
    if (millis() - lastUpdate >= 2000) {
        lastUpdate = millis();
        
        float testPh = 7.0 + (counter % 10) * 0.5;
        float testTemp = 25.0 + (counter % 5);
        
        // Use new API with test output states
        _lcd->showAutoManualScreen(testPh, testTemp, false, false, false, false, true);
        Serial.printf("[LCD Test] pH=%.1f | Temp=%.1f\n", testPh, testTemp);
        
        counter++;
    }
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
                
                // Log context
                Serial.printf("[Context] pH=%.2f | Temp=%.1f | Mode=AUTO | Out=%d%d%d%d\n",
                    _context.currentPh, _context.currentTemp,
                    _context.output1, _context.output2, _context.output3, _context.output4);
                
                runAutoLogic();
                updateOutputs();
            }
            break;
            
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

    // --- Simulated pH Pattern ---
    // Cycle: Phase 0 (below lower, rising) -> Phase 1 (in range) 
    //     -> Phase 2 (above upper, falling) -> Phase 3 (in range) -> repeat
    static int sampleIndex = 0;
    const int SAMPLES_PER_PHASE = 5;
    const int TOTAL_PHASES = 4;
    const int TOTAL_SAMPLES = SAMPLES_PER_PHASE * TOTAL_PHASES; // 20 samples per cycle
    
    int phase = (sampleIndex / SAMPLES_PER_PHASE) % TOTAL_PHASES;
    int stepInPhase = sampleIndex % SAMPLES_PER_PHASE; // 0-4
    
    float lower = _config.phLowerLimit;
    float upper = _config.phUpperLimit;
    float midpoint = (lower + upper) / 2.0f;
    
    float phValue = midpoint; // Default fallback
    
    switch (phase) {
        case 0: {
            // Phase 0: Below lower threshold, rising toward lower (but not reaching)
            // Start at lower - 1.5, end at lower - 0.3
            float startPh = lower - 1.5f;
            float endPh = lower - 0.3f;
            phValue = startPh + (endPh - startPh) * (stepInPhase / 4.0f);
            break;
        }
        case 1: {
            // Phase 1: Within range (lower to upper)
            // Smooth transition from near lower to midpoint
            float startPh = lower + 0.2f;
            float endPh = midpoint + 0.3f;
            phValue = startPh + (endPh - startPh) * (stepInPhase / 4.0f);
            break;
        }
        case 2: {
            // Phase 2: Above upper threshold, falling toward upper (but not reaching)
            // Start at upper + 1.5, end at upper + 0.3
            float startPh = upper + 1.5f;
            float endPh = upper + 0.3f;
            phValue = startPh + (endPh - startPh) * (stepInPhase / 4.0f);
            break;
        }
        case 3: {
            // Phase 3: Within range (back in normal zone)
            // Smooth transition from near upper to midpoint
            float startPh = upper - 0.2f;
            float endPh = midpoint - 0.3f;
            phValue = startPh + (endPh - startPh) * (stepInPhase / 4.0f);
            break;
        }
    }
    
    // Clamp to valid pH range (0-14)
    if (phValue < 0.0f) phValue = 0.0f;
    if (phValue > 14.0f) phValue = 14.0f;
    
    _context.currentPh = phValue;
    
    // // Log phase info for debugging
    // const char* phaseNames[] = {"BELOW_LOWER", "IN_RANGE_1", "ABOVE_UPPER", "IN_RANGE_2"};
    // Serial.printf("[Sensor] Sample %d | Phase: %s | Step: %d | pH: %.2f\n", 
    //               sampleIndex, phaseNames[phase], stepInPhase, phValue);
    
    // Advance to next sample (wrap around)
    sampleIndex = (sampleIndex + 1) % TOTAL_SAMPLES;
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
    // In Config Mode, we read Upper Potentiometer only
    // Lower threshold is fixed at 4.5
    // Upper is always snapped to nearest 0.5 milestone
    if (_context.configState == CFG_THRESHOLD) {
         float valUpper = _potUpper->getScaledValue(0, 100); // 0-100
         
         // Map to pH range 0-14
         float rawUpper = (valUpper / 100.0f) * 14.0f;
         
         // Always snap to nearest 0.5 milestone (0.0, 0.5, 1.0, 1.5, ...)
         float snappedUpper = round(rawUpper * 2.0f) / 2.0f;
         
         // Fixed lower threshold
         const float FIXED_LOWER = 4.5f;
         const float MIN_GAP = 0.5f; // Minimum gap between upper and lower
         
         // Enforce upper >= lower + MIN_GAP
         float minUpper = FIXED_LOWER + MIN_GAP;
         if (snappedUpper < minUpper) {
             snappedUpper = minUpper;
         }
         
         // Clamp to valid pH range
         if (snappedUpper > 14.0f) snappedUpper = 14.0f;
         
         // Only update and log if value changed
         static float lastUp = -1.0;
         
         if (snappedUpper != lastUp) {
             _config.phUpperLimit = snappedUpper;
             _config.phLowerLimit = FIXED_LOWER;
             Serial.printf("[Config] Upper: %.1f | Lower: %.1f (fixed)\n", snappedUpper, FIXED_LOWER);
             lastUp = snappedUpper;
         }
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
    bool outputChanged = (_context.output1 != _lastContext.output1) ||
                         (_context.output2 != _lastContext.output2) ||
                         (_context.output3 != _lastContext.output3) ||
                         (_context.output4 != _lastContext.output4);
    
    // Force update on first call, mode change, value change, or output change
    bool needUpdate = _forceDisplayUpdate || modeChanged || configStateChanged || valueChanged || outputChanged;
    
    // For INFO mode, always update (it cycles pages internally)
    if (_context.systemMode == MODE_INFOR) {
        needUpdate = true;
    }
    
    // For CONFIG mode, always update (config values may change continuously)
    if (_context.systemMode == MODE_CONFIG) {
        needUpdate = true;
    }
    
    if (needUpdate) {
        _forceDisplayUpdate = false; // Clear force flag
        
        switch (_context.systemMode) {
            case MODE_AUTO:
                _lcd->showAutoManualScreen(
                    _context.currentPh, _context.currentTemp,
                    _context.output1, _context.output2, _context.output3, _context.output4,
                    true  // isAuto = true
                );
                break;
                
            case MODE_MANUAL:
                _lcd->showAutoManualScreen(
                    _context.currentPh, _context.currentTemp,
                    _context.output1, _context.output2, _context.output3, _context.output4,
                    false  // isAuto = false
                );
                break;
                
            case MODE_CONFIG:
                switch (_context.configState) {
                    case CFG_THRESHOLD:
                        _lcd->showConfigScreen(CFG_THRESHOLD, _config.phUpperLimit, _config.phLowerLimit);
                        break;
                    case CFG_SLOPE:
                        _lcd->showConfigScreen(CFG_SLOPE, _config.calibSlope, 0);
                        break;
                    case CFG_INTERCEPT:
                        _lcd->showConfigScreen(CFG_INTERCEPT, _config.calibIntercept, 0);
                        break;
                }
                break;
                
            case MODE_INFOR:
                _lcd->showInfoScreen(_config.phUpperLimit, _config.phLowerLimit, 
                                     _config.calibSlope, _config.calibIntercept);
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

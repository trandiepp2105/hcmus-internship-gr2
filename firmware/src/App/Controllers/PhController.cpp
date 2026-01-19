#include "PhController.h"
#include "../../middleware/bsp_board.h" // For pin defs if needed, though passed via Dependency Injection

// Static pointer for callback access
static PhController* g_phController = nullptr;

// Callback function for control mode changes from MQTT
static void onControlModeChange(bool isAuto) {
    if (g_phController) {
        g_phController->setControlMode(isAuto);
    }
}

// Callback function for RPC commands from MQTT
static bool onRpcCommand(const char* method, int relay, bool state) {
    if (g_phController) {
        return g_phController->handleRpcSetRelay(relay, state);
    }
    return false;
}

// Callback function for MQTT connect event
static void onMqttConnect() {
    if (g_phController) {
        g_phController->syncControlMode();
        g_phController->syncThresholds();
    }
}

// Callback function for threshold changes from ThingsBoard
static void onThresholdChange(float minThreshold, float maxThreshold) {
    if (g_phController) {
        g_phController->handleThresholdUpdate(minThreshold, maxThreshold);
    }
}

PhController::PhController(Storage* storage, 
                           // IOExpanderBSP* ioExpander,
                           ButtonHandler* btnMode, 
                           ButtonHandler* btnThreshold,
                           ButtonHandler* btnCalib,
                           TftHandler* tft,
                           WifiHandler* wifi,
                           PotHandler* potUpper,
                           PotHandler* potLower,
                           TempSensorHandler* tempSensor,
                           RelayHandler* relayHandler,
                           MqttHandler* mqttHandler)
    : _storage(storage), 
      // _ioExpander(ioExpander), 
      _btnMode(btnMode), 
      _btnThreshold(btnThreshold), 
      _btnCalib(btnCalib),
      _tft(tft),
      _wifi(wifi),
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
    
    // 4. Register MQTT callbacks
    g_phController = this;
    if (_mqttHandler) {
        _mqttHandler->setControlModeCallback(onControlModeChange);
        _mqttHandler->setRpcCallback(onRpcCommand);
        _mqttHandler->setOnConnectCallback(onMqttConnect);
        _mqttHandler->setThresholdCallback(onThresholdChange);
    }
    
    Serial.println("[PhController] Started.");
}

void PhController::setControlMode(bool isAuto) {
    if (isAuto) {
        _context.systemMode = MODE_AUTO;
        _context.isAutoControl = true;
        Serial.println("[PhController] Switched to AUTO mode via MQTT");
    } else {
        _context.systemMode = MODE_MANUAL;
        _context.isAutoControl = false;
        stopAllActuators();  // Turn off all relays when entering MANUAL
        Serial.println("[PhController] Switched to MANUAL mode via MQTT (all relays OFF)");
    }
    
    // Reset display for mode change
    _tft->resetOnModeChange();
    
    // Sync to ThingsBoard
    if (_mqttHandler && _mqttHandler->isConnected()) {
        _mqttHandler->pushAttribute("control_mode", isAuto);
    }
}

void PhController::syncControlMode() {
    // Push current control mode to ThingsBoard
    if (_mqttHandler && _mqttHandler->isConnected()) {
        bool isAuto = (_context.systemMode == MODE_AUTO);
        _mqttHandler->pushAttribute("control_mode", isAuto);
        Serial.printf("[PhController] Synced control_mode to ThingsBoard: %s\n", isAuto ? "AUTO" : "MANUAL");
    }
}

void PhController::syncThresholds() {
    // Push current thresholds to ThingsBoard
    if (_mqttHandler && _mqttHandler->isConnected()) {
        _mqttHandler->pushAttribute("min_threshold", _config.phLowerLimit);
        _mqttHandler->pushAttribute("max_threshold", _config.phUpperLimit);
        Serial.printf("[PhController] Synced thresholds: min=%.1f, max=%.1f\n", 
                      _config.phLowerLimit, _config.phUpperLimit);
    }
}

void PhController::handleThresholdUpdate(float minThreshold, float maxThreshold) {
    bool updated = false;
    
    // Update min_threshold (phLowerLimit) if valid
    if (minThreshold >= 0.0f && minThreshold <= 14.0f) {
        _config.phLowerLimit = minThreshold;
        updated = true;
        Serial.printf("[PhController] Updated phLowerLimit = %.1f from ThingsBoard\n", minThreshold);
    }
    
    // Update max_threshold (phUpperLimit) if valid
    if (maxThreshold >= 0.0f && maxThreshold <= 14.0f) {
        _config.phUpperLimit = maxThreshold;
        updated = true;
        Serial.printf("[PhController] Updated phUpperLimit = %.1f from ThingsBoard\n", maxThreshold);
    }
    
    // Save to persistent storage if updated
    if (updated) {
        saveConfig();
        Serial.println("[PhController] Thresholds saved to storage");
    }
}

bool PhController::handleRpcSetRelay(int relay, bool state) {
    // Only allow relay control in MANUAL mode
    if (_context.systemMode != MODE_MANUAL) {
        Serial.println("[PhController] RPC REJECTED: Not in MANUAL mode");
        return false;
    }
    
    // relay = 0 means ALL relays
    if (relay == 0) {
        _context.output1 = state;
        _context.output2 = state;
        _context.output3 = state;
        _context.output4 = state;
        
        if (_relayHandler) {
            _relayHandler->setRelay(1, state);
            _relayHandler->setRelay(2, state);
            _relayHandler->setRelay(3, state);
            _relayHandler->setRelay(4, state);
        }
        
        Serial.printf("[PhController] RPC OK: ALL Relays = %s\n", state ? "ON" : "OFF");
        return true;
    }
    
    // Validate relay number (1-4)
    if (relay < 1 || relay > 4) {
        Serial.printf("[PhController] RPC REJECTED: Invalid relay %d\n", relay);
        return false;
    }
    
    // Update context output
    switch (relay) {
        case 1: _context.output1 = state; break;
        case 2: _context.output2 = state; break;
        case 3: _context.output3 = state; break;
        case 4: _context.output4 = state; break;
    }
    
    // Apply to hardware
    if (_relayHandler) {
        _relayHandler->setRelay(relay, state);
    }
    
    Serial.printf("[PhController] RPC OK: Relay %d = %s\n", relay, state ? "ON" : "OFF");
    return true;
}

void PhController::testLcd() {
    static int counter = 0;
    static unsigned long lastUpdate = 0;
    
    // Update every 2 seconds
    if (millis() - lastUpdate >= 2000) {
        lastUpdate = millis();
        
        float testPh = 7.0 + (counter % 10) * 0.5;
        float testTemp = 25.0 + (counter % 5);
        
        // Use new TFT API with test output states
        _tft->showAutoManualScreen(testPh, testTemp, false, false, false, false, true, 8.5, 6.5);
        Serial.printf("[TFT Test] pH=%.1f | Temp=%.1f\n", testPh, testTemp);
        
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

                // Send telemetry if MQTT connected
                if (_mqttHandler && _mqttHandler->isConnected()) {
                    uint8_t relayMask = 0;
                    if (_context.output1) relayMask |= 0x01;
                    if (_context.output2) relayMask |= 0x02;
                    if (_context.output3) relayMask |= 0x04;
                    if (_context.output4) relayMask |= 0x08;

                    _mqttHandler->pushTelemetry(
                        _context.currentPh, 
                        _context.currentTemp, 
                        relayMask, 
                        _context.systemMode, 
                        0
                    );
                }
                
                // These should ALWAYS run, not just when MQTT connected!
                Serial.printf("[Context] pH=%.2f | Temp=%.1f | Mode=AUTO | Out=%d%d%d%d\n",
                    _context.currentPh, _context.currentTemp,
                    _context.output1, _context.output2, _context.output3, _context.output4);
                
                runAutoLogic();
                updateOutputs();
                updateDisplay();
            }
            break;
        case MODE_MANUAL:
            // Periodic Tasks - Every 5 Seconds
            if (millis() - _lastSampleTime >= PH_SAMPLE_INTERVAL_MS) {
                _lastSampleTime = millis();
                readSensors();
                
                // Send telemetry if MQTT connected (same as AUTO mode)
                if (_mqttHandler && _mqttHandler->isConnected()) {
                    uint8_t relayMask = 0;
                    if (_context.output1) relayMask |= 0x01;
                    if (_context.output2) relayMask |= 0x02;
                    if (_context.output3) relayMask |= 0x04;
                    if (_context.output4) relayMask |= 0x08;

                    _mqttHandler->pushTelemetry(
                        _context.currentPh, 
                        _context.currentTemp, 
                        relayMask, 
                        _context.systemMode, 
                        0
                    );
                }
                
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
    // --- Button A: Mode Switching or Cancel Config ---
    if (_btnMode->checkClicked()) {
        Serial.println("[Input] Button A Pressed");
        
        if (_context.systemMode == MODE_CONFIG) {
            // In CONFIG mode: Button A cancels without saving
            Serial.println("[Input] Config CANCELLED -> INFO (no save)");
            _context.systemMode = MODE_INFOR;
            _context.isAutoControl = false;
            stopAllActuators();
            
            // Reset TFT and update display
            _tft->resetOnModeChange();
            updateDisplay();
            return;
        }
        
        // Normal mode cycling (not in CONFIG): AUTO -> MANUAL -> INFO -> AUTO
        switch (_context.systemMode) {
            case MODE_AUTO:
                _context.systemMode = MODE_MANUAL;
                _context.isAutoControl = false;
                Serial.println("[Input] AUTO -> MANUAL");
                break;
                
            case MODE_MANUAL:
                _context.systemMode = MODE_INFOR;
                _context.isAutoControl = false;
                stopAllActuators();
                Serial.println("[Input] MANUAL -> INFO");
                break;
                
            case MODE_INFOR:
                _context.systemMode = MODE_AUTO;
                _context.isAutoControl = true;
                Serial.println("[Input] INFO -> AUTO");
                break;
                
            case MODE_CONFIG:
                // Already handled above
                break;
        }
        
        // Reset TFT state to force full redraw on mode change
        _tft->resetOnModeChange();
        
        // IMMEDIATE display update
        updateDisplay();
        
        // Sync control_mode to ThingsBoard
        if (_mqttHandler && _mqttHandler->isConnected()) {
            bool isAuto = (_context.systemMode == MODE_AUTO);
            _mqttHandler->pushAttribute("control_mode", isAuto);
        }
    }

    // --- Button B: Enter/Save CONFIG or Next Config Page ---
    if (_btnThreshold->checkClicked()) {
        Serial.println("[Input] Button B Pressed");
        
        if (_context.systemMode == MODE_INFOR) {
            // Enter CONFIG_THRESHOLD from INFO
            Serial.println("[Input] INFO -> CONFIG (THRESHOLD)");
            _context.systemMode = MODE_CONFIG;
            _context.configState = CFG_THRESHOLD;
            _context.isAutoControl = false;
            stopAllActuators();
            
            _tft->resetOnModeChange();
            updateDisplay();
            
        } else if (_context.systemMode == MODE_CONFIG) {
            // In CONFIG mode: Button B saves and returns to INFO
            Serial.println("[Input] CONFIG -> INFO (SAVE)");
            
            saveConfig();        // Save to storage
            syncThresholds();    // Sync to ThingsBoard
            
            _context.systemMode = MODE_INFOR;
            _context.isAutoControl = false;
            
            _tft->resetOnModeChange();
            updateDisplay();
        }
    }
    
    // --- Button C: Enter/Save CALIB Config ---
    if (_btnCalib->checkClicked()) {
        Serial.println("[Input] Button C Pressed");
        
        if (_context.systemMode == MODE_INFOR) {
            // Enter CONFIG_CALIB from INFO (Slope/Intercept)
            Serial.println("[Input] INFO -> CONFIG (CALIB)");
            _context.systemMode = MODE_CONFIG;
            _context.configState = CFG_SLOPE;  // Start with slope
            _context.isAutoControl = false;
            stopAllActuators();
            
            _tft->resetOnModeChange();
            updateDisplay();
            
        } else if (_context.systemMode == MODE_CONFIG && 
                   (_context.configState == CFG_SLOPE || _context.configState == CFG_INTERCEPT)) {
            // In CONFIG_CALIB mode: Button C saves and returns to INFO
            Serial.println("[Input] CONFIG (CALIB) -> INFO (SAVE)");
            
            saveConfig();        // Save calibration to storage
            syncThresholds();    // Sync to ThingsBoard
            
            _context.systemMode = MODE_INFOR;
            _context.isAutoControl = false;
            
            _tft->resetOnModeChange();
            updateDisplay();
        }
    }
}

void PhController::handleButtonEvent(uint8_t button) {
    Serial.printf("[PhController] Button %c event\n", 'A' + button);
    
    if (button == 0) { // BUTTON_A
        if (_context.systemMode == MODE_CONFIG) {
            Serial.println("[Input] Config CANCELLED -> INFO");
            _context.systemMode = MODE_INFOR;
            _context.isAutoControl = false;
            stopAllActuators();
        } else {
            switch (_context.systemMode) {
                case MODE_AUTO:
                    _context.systemMode = MODE_MANUAL;
                    _context.isAutoControl = false;
                    Serial.println("[Input] AUTO -> MANUAL");
                    break;
                case MODE_MANUAL:
                    _context.systemMode = MODE_INFOR;
                    _context.isAutoControl = false;
                    stopAllActuators();
                    Serial.println("[Input] MANUAL -> INFO");
                    break;
                case MODE_INFOR:
                    _context.systemMode = MODE_AUTO;
                    _context.isAutoControl = true;
                    Serial.println("[Input] INFO -> AUTO");
                    break;
                default: break;
            }
        }
        _tft->resetOnModeChange();
        updateDisplay();
        if (_mqttHandler && _mqttHandler->isConnected()) {
            _mqttHandler->pushAttribute("control_mode", _context.systemMode == MODE_AUTO);
        }
    }
    else if (button == 1) { // BUTTON_B  
        if (_context.systemMode == MODE_INFOR) {
            Serial.println("[Input] INFO -> CONFIG (THRESHOLD)");
            _context.systemMode = MODE_CONFIG;
            _context.configState = CFG_THRESHOLD;
            _context.isAutoControl = false;
            stopAllActuators();
            _tft->resetOnModeChange();
            updateDisplay();
        } else if (_context.systemMode == MODE_CONFIG && _context.configState == CFG_THRESHOLD) {
            Serial.println("[Input] CONFIG -> INFO (SAVE)");
            saveConfig();
            syncThresholds();
            _context.systemMode = MODE_INFOR;
            _tft->resetOnModeChange();
            updateDisplay();
        }
    }
    else if (button == 2) { // BUTTON_C
        if (_context.systemMode == MODE_INFOR) {
            Serial.println("[Input] INFO -> CONFIG (CALIB)");
            _context.systemMode = MODE_CONFIG;
            _context.configState = CFG_SLOPE;
            _context.isAutoControl = false;
            stopAllActuators();
            _tft->resetOnModeChange();
            updateDisplay();
        } else if (_context.systemMode == MODE_CONFIG && 
                   (_context.configState == CFG_SLOPE || _context.configState == CFG_INTERCEPT)) {
            Serial.println("[Input] CONFIG (CALIB) -> INFO (SAVE)");
            saveConfig();
            syncThresholds();
            _context.systemMode = MODE_INFOR;
            _tft->resetOnModeChange();
            updateDisplay();
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
    // In Config Mode, we read both Upper and Lower Potentiometers
    // Both values are snapped to nearest 0.5 milestone
    if (_context.configState == CFG_THRESHOLD) {
         float valUpper = _potUpper->getScaledValue(0, 100); // 0-100
         float valLower = _potLower->getScaledValue(0, 100); // 0-100
         
         // Map to pH range 0-14
         float rawUpper = (valUpper / 100.0f) * 14.0f;
         float rawLower = (valLower / 100.0f) * 14.0f;
         
         // Always snap to nearest 0.5 milestone (0.0, 0.5, 1.0, 1.5, ...)
         float snappedUpper = round(rawUpper * 2.0f) / 2.0f;
         float snappedLower = round(rawLower * 2.0f) / 2.0f;
         
         const float MIN_GAP = 0.5f; // Minimum gap between upper and lower
         
         // Enforce upper >= lower + MIN_GAP
         if (snappedUpper < snappedLower + MIN_GAP) {
             snappedUpper = snappedLower + MIN_GAP;
         }
         
         // Clamp to valid pH range
         if (snappedLower < 0.0f) snappedLower = 0.0f;
         if (snappedUpper > 14.0f) snappedUpper = 14.0f;
         
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
                _tft->showAutoManualScreen(
                    _context.currentPh, _context.currentTemp,
                    _context.output1, _context.output2, _context.output3, _context.output4,
                    true,  // isAuto = true
                    _config.phUpperLimit, _config.phLowerLimit
                );
                break;
                
            case MODE_MANUAL:
                _tft->showAutoManualScreen(
                    _context.currentPh, _context.currentTemp,
                    _context.output1, _context.output2, _context.output3, _context.output4,
                    false,  // isAuto = false
                    _config.phUpperLimit, _config.phLowerLimit
                );
                break;
                
            case MODE_CONFIG:
                switch (_context.configState) {
                    case CFG_THRESHOLD:
                        _tft->showConfigScreen(CFG_THRESHOLD, _config.phUpperLimit, _config.phLowerLimit);
                        break;
                    case CFG_SLOPE:
                        _tft->showConfigScreen(CFG_SLOPE, _config.calibSlope, 0);
                        break;
                    case CFG_INTERCEPT:
                        _tft->showConfigScreen(CFG_INTERCEPT, _config.calibIntercept, 0);
                        break;
                }
                break;
                
            case MODE_INFOR: {
                // Get connection status
                bool wifiOk = _wifi ? _wifi->isConnected() : false;
                bool mqttOk = _mqttHandler ? _mqttHandler->isConnected() : false;
                String ip = _wifi ? _wifi->getIP() : "N/A";
                
                // Determine next mode name
                String nextMode = "AUTO";  // INFO -> AUTO
                
                _tft->showInfoScreen(
                    wifiOk, mqttOk,
                    ip, TB_DEVICE_NAME,
                    _config.phUpperLimit, _config.phLowerLimit,
                    _config.calibSlope, _config.calibIntercept,
                    nextMode
                );
                break;
            }
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

void PhController::factoryReset() {
    Serial.println("====================================");
    Serial.println("       FACTORY RESET TRIGGERED!");
    Serial.println("====================================");
    Serial.println("Clearing ALL storage data...");
    
    // Stop all actuators first
    stopAllActuators();
    
    // Clear app config storage
    _storage->clear();
    Serial.println("[Factory Reset] App config cleared");
    
    // Clear WiFi credentials using separate Preferences namespace
    Preferences wifiPrefs;
    wifiPrefs.begin("wifi-config", false);
    wifiPrefs.clear();
    wifiPrefs.end();
    Serial.println("[Factory Reset] WiFi credentials cleared");
    
    // Clear any other namespaces that might exist
    Preferences mqttPrefs;
    mqttPrefs.begin("mqtt-config", false);
    mqttPrefs.clear();
    mqttPrefs.end();
    Serial.println("[Factory Reset] MQTT config cleared");
    
    // Show reset message on display
    _tft->resetOnModeChange();
    // Can't call full updateDisplay here, just show message
    
    Serial.println("====================================");
    Serial.println("  Factory reset complete!");
    Serial.println("  Restarting in 2 seconds...");
    Serial.println("====================================");
    
    delay(2000);
    
    // Restart ESP32 to apply changes
    ESP.restart();
}

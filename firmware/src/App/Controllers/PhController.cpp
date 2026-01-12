#include "PhController.h"
#include "../../middleware/bsp_board.h" // For pin defs if needed, though passed via Dependency Injection

PhController::PhController(Storage *storage,
                           // IOExpanderBSP* ioExpander,
                           ButtonHandler *btnA,
                           ButtonHandler *btnB,
                           LcdHandler *lcd,
                           PotHandler *potUpper,
                           PotHandler *potLower,
                           TempSensorHandler *tempSensor,
                           RelayHandler *relayHandler)
    : _storage(storage),
      // _ioExpander(ioExpander),
      _btnA(btnA),
      _btnB(btnB),
      _lcd(lcd),
      _potUpper(potUpper),
      _potLower(potLower),
      _tempSensor(tempSensor),
      _relayHandler(relayHandler)
{
}

void PhController::begin()
{
    // 1. Load Config
    loadConfig();

    // 2. Init Hardware (Drivers usually init outside or here if specific logic needed)
    // Assuming Drivers are .begin() call in main or SystemController init

    // 3. Set Initial State
    _context.systemMode = MODE_AUTO;
    _context.isAutoControl = true;

    Serial.println("[PhController] Started.");
}

void PhController::update()
{
    // 1. Handle Button Inputs (Always responsive)
    handleInputs();

    // 2. Mode-specific Logic
    switch (_context.systemMode)
    {
    case MODE_AUTO:
        // Periodic Tasks - Every 5 Seconds
        if (millis() - _lastSampleTime >= PH_SAMPLE_INTERVAL_MS)
        {
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
        if (millis() - _lastSampleTime >= PH_SAMPLE_INTERVAL_MS)
        {
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

void PhController::readSensors()
{
    float tempValue = _tempSensor->getTemperature();
    TempError_t status = _tempSensor->getErrorStatus();

    switch (status)
    {
    case ERR_NONE:
        _context.currentTemp = tempValue;
        _lcd->print(0, 1, "Temp: OK    ");
        break;

    case ERR_DISCONNECTED:
        _lcd->print(0, 1, "ERR: NO WIRE");
        // ĐỔI TÊN Ở ĐÂY: Từ _relays thành _relayHandler
        _relayHandler->allOff();
        break;

    case ERR_OUT_OF_RANGE:
        _lcd->print(0, 1, "ERR: OVER T ");
        _relayHandler->allOff();
        break;

    default:
        _lcd->print(0, 1, "ERR: UNKNOWN");
        break;
    }
}

void PhController::handleInputs()
{
    // --- Button A: Mode Switching ---
    if (_btnA->checkClicked())
    { // Assuming isPressed handles debounce and returns true once on press
        Serial.println("[Input] Button A Pressed -> Changing Mode");
        switch (_context.systemMode)
        {
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
    if (_btnB->checkClicked())
    {
        Serial.println("[Input] Button B Pressed");
        if (_context.systemMode == MODE_CONFIG)
        {
            // Cycle Config Pages
            switch (_context.configState)
            {
            case CFG_THRESHOLD:
                _context.configState = CFG_SLOPE;
                break;
            case CFG_SLOPE:
                _context.configState = CFG_INTERCEPT;
                break;
            case CFG_INTERCEPT:
                _context.configState = CFG_THRESHOLD;
                break;
            }
        }
    }
}

void PhController::runAutoLogic()
{
    // Control Logic:
    // pH > Upper: Output 2 (ACID) & 4 (ACID) -> ON
    // pH < Lower: Output 1 (BASE) & 3 (BASE) -> ON
    // Lower <= pH <= Upper: ALL OFF

    if (_context.currentPh > _config.phUpperLimit)
    {
        _context.output1 = false; // Base OFF
        _context.output2 = true;  // Acid ON
        _context.output3 = false; // Base OFF
        _context.output4 = true;  // Acid ON
    }
    else if (_context.currentPh < _config.phLowerLimit)
    {
        _context.output1 = true;  // Base ON
        _context.output2 = false; // Acid OFF
        _context.output3 = true;  // Base ON
        _context.output4 = false; // Acid OFF
    }
    else
    {
        _context.output1 = false;
        _context.output2 = false;
        _context.output3 = false;
        _context.output4 = false; // All OK
    }
}

void PhController::runManualLogic()
{
    // Do nothing logic-wise, outputs stay as last set OR controlled via Remote (ThingsBoard) implementation later
    // For safety, ensure we don't accidentally toggle pumps here.
}

void PhController::runConfigLogic()
{
    // In Config Mode, we read Potentiometers and update Config IMMEDIATELY (Live Preview)
    if (_context.configState == CFG_THRESHOLD)
    {
        float valUpper = _potUpper->getScaledValue(0, 100); // 0-100
        float valLower = _potLower->getScaledValue(0, 100); // 0-100

        _config.phUpperLimit = (valUpper / 100.0f) * 14.0f;
        _config.phLowerLimit = (valLower / 100.0f) * 14.0f;

        // Log values only when they change (prevent spam)
        static float lastUp = -1.0;
        static float lastLow = -1.0;
        if (abs(_config.phUpperLimit - lastUp) > 0.05 || abs(_config.phLowerLimit - lastLow) > 0.05)
        {
            Serial.printf("[Config] Upper: %.2f | Lower: %.2f\n", _config.phUpperLimit, _config.phLowerLimit);
            lastUp = _config.phUpperLimit;
            lastLow = _config.phLowerLimit;
        }
    }
}

void PhController::runInforLogic()
{
    // Logic for Info Mode
    // The LCD update happens in updateDisplay().
    // We can add a periodic log here to confirm values.
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 2000)
    {
        Serial.printf("[Info] Config in Memory: Upper=%.2f, Lower=%.2f\n",
                      _config.phUpperLimit, _config.phLowerLimit);
        lastLog = millis();
    }
}

void PhController::updateDisplay()
{
    // Check if any relevant value changed
    bool modeChanged = (_context.systemMode != _lastContext.systemMode);
    bool configStateChanged = (_context.configState != _lastContext.configState);
    bool valueChanged = (abs(_context.currentPh - _lastContext.currentPh) > 0.01) ||
                        (abs(_context.currentTemp - _lastContext.currentTemp) > 0.3);

    // Force update on first call, mode change, or value change
    bool needUpdate = _forceDisplayUpdate || modeChanged || configStateChanged || valueChanged;

    if (needUpdate)
    {
        _forceDisplayUpdate = false; // Clear force flag

        switch (_context.systemMode)
        {
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

void PhController::updateOutputs()
{
    // Check if any output state changed
    bool changed = (_context.output1 != _lastContext.output1) ||
                   (_context.output2 != _lastContext.output2) ||
                   (_context.output3 != _lastContext.output3) ||
                   (_context.output4 != _lastContext.output4);

    if (changed)
    {
        // Build bitmask for 8-relay module (only use bits 0-3, bits 4-7 = 0)
        uint8_t relayMask = 0;
        if (_context.output1)
            relayMask |= 0x01; // Bit 0 = Relay 1 (BASE)
        if (_context.output2)
            relayMask |= 0x02; // Bit 1 = Relay 2 (ACID)
        if (_context.output3)
            relayMask |= 0x04; // Bit 2 = Relay 3 (BASE)
        if (_context.output4)
            relayMask |= 0x08; // Bit 3 = Relay 4 (ACID)
        // Bits 4-7 remain 0

        // Drive actual relays
        if (_relayHandler)
        {
            _relayHandler->setAllRelays(relayMask);
        }

        Serial.print("[Output] State Changed: ");
        if (_context.output1)
            Serial.print("BASE(1)=ON ");
        else
            Serial.print("BASE(1)=OFF ");
        if (_context.output2)
            Serial.print("ACID(2)=ON ");
        else
            Serial.print("ACID(2)=OFF ");
        if (_context.output3)
            Serial.print("BASE(3)=ON ");
        else
            Serial.print("BASE(3)=OFF ");
        if (_context.output4)
            Serial.print("ACID(4)=ON");
        else
            Serial.print("ACID(4)=OFF");
        Serial.println();
    }

    // Note: Tracking update moved to main update() loop
}

void PhController::stopAllActuators()
{
    _context.output1 = false;
    _context.output2 = false;
    _context.output3 = false;
    _context.output4 = false;
    updateOutputs(); // Apply immediately
}

void PhController::loadConfig()
{
    // Load from Storage
    _config.phUpperLimit = _storage->getDouble("ph_upper", 8.5);
    _config.phLowerLimit = _storage->getDouble("ph_lower", 6.5);
    _config.calibSlope = _storage->getDouble("calib_slope", 1.0);
    _config.calibIntercept = _storage->getDouble("calib_int", 0.0);
}

void PhController::saveConfig()
{
    // Save to Storage
    _storage->putDouble("ph_upper", _config.phUpperLimit);
    _storage->putDouble("ph_lower", _config.phLowerLimit);
    _storage->putDouble("calib_slope", _config.calibSlope);
    _storage->putDouble("calib_int", _config.calibIntercept);
    Serial.println("[Config] Saved.");
}

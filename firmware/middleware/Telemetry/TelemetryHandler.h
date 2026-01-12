#ifndef TELEMETRY_HANDLER_H
#define TELEMETRY_HANDLER_H

#include "../../lib/TelemetryData/TelemetryQueueDriver.h"

/**
 * @class TelemetryHandler
 * @brief BSP/Middleware for managing Telemetry Data storage
 */
class TelemetryHandler {
public:
    TelemetryHandler();
    void begin(); // Init

    /**
     * @brief Save current system state to queue (RAM)
     * @param ph Current pH
     * @param temp Current Temperature
     * @param out1..4 Output States
     * @param mode System Mode
     * @param err Error Code (0 = OK)
     */
    void logData(float ph, float temp, bool out1, bool out2, bool out3, bool out4, uint8_t mode, uint8_t err);

    /**
     * @brief Retrieve oldest record to send via MQTT
     * @param record Output reference
     * @return true if data exists, false if empty
     */
    bool getNextData(TelemetryRecord& record);

    bool hasData();
    uint16_t dataCount();

private:
    TelemetryQueueDriver _queue;
    uint8_t packOutputs(bool o1, bool o2, bool o3, bool o4);
};

#endif

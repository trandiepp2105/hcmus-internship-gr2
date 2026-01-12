#ifndef TELEMETRY_QUEUE_H
#define TELEMETRY_QUEUE_H

#include <Arduino.h>
#include "TelemetryRecord.h"

// Define capacity. dynamic or static? Static is safer for fragmentation.
// 500 records * 16 bytes = ~8KB. Safe for ESP8266/ESP32.
#define TELEMETRY_QUEUE_SIZE 500 

/**
 * @class TelemetryQueueDriver
 * @brief Low-level Circular Buffer for Telemetry Records
 */
class TelemetryQueueDriver {
public:
    TelemetryQueueDriver();

    bool push(const TelemetryRecord& record);
    bool pop(TelemetryRecord& record);
    bool peek(TelemetryRecord& record);
    
    bool isEmpty() const;
    bool isFull() const;
    uint16_t count() const;
    void clear();

private:
    TelemetryRecord _buffer[TELEMETRY_QUEUE_SIZE];
    uint16_t _head;
    uint16_t _tail;
    uint16_t _count;
};

#endif

#ifndef TELEMETRY_RECORD_H
#define TELEMETRY_RECORD_H

#include <Arduino.h>

struct TelemetryRecord {
    unsigned long timestamp; // millis() or epoch
    float ph;
    float temp;
    uint8_t outputs; // Bitmask: Bit 0->Out1, 1->Out2...
    uint8_t mode;    // 0: Auto, 1: Manual, 2: Config...
    uint8_t errorCode; 
};

#endif

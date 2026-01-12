#include "TelemetryHandler.h"

TelemetryHandler::TelemetryHandler() {
}

void TelemetryHandler::begin() {
    _queue.clear();
}

void TelemetryHandler::logData(float ph, float temp, bool out1, bool out2, bool out3, bool out4, uint8_t mode, uint8_t err) {
    TelemetryRecord rec;
    rec.timestamp = millis();
    rec.ph = ph;
    rec.temp = temp;
    rec.outputs = packOutputs(out1, out2, out3, out4);
    rec.mode = mode;
    rec.errorCode = err;

    _queue.push(rec);
    
    // Optional debug
    // Serial.printf("[Telem] Saved RAM. Count: %d\n", _queue.count());
}

bool TelemetryHandler::getNextData(TelemetryRecord& record) {
    return _queue.pop(record);
}

bool TelemetryHandler::hasData() {
    return !_queue.isEmpty();
}

uint16_t TelemetryHandler::dataCount() {
    return _queue.count();
}

uint8_t TelemetryHandler::packOutputs(bool o1, bool o2, bool o3, bool o4) {
    uint8_t mask = 0;
    if (o1) mask |= 0x01; // Bit 0
    if (o2) mask |= 0x02; // Bit 1
    if (o3) mask |= 0x04; // Bit 2
    if (o4) mask |= 0x08; // Bit 3
    return mask;
}

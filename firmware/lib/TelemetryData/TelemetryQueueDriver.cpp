#include "TelemetryQueueDriver.h"

TelemetryQueueDriver::TelemetryQueueDriver() {
    clear();
}

void TelemetryQueueDriver::clear() {
    _head = 0;
    _tail = 0;
    _count = 0;
}

bool TelemetryQueueDriver::push(const TelemetryRecord& record) {
    // Save record
    _buffer[_head] = record;
    
    // Move Head
    _head = (_head + 1) % TELEMETRY_QUEUE_SIZE;

    // Logic: If Full, we Overwrite. So Tail must also move (drop oldest).
    if (_count == TELEMETRY_QUEUE_SIZE) {
        // Queue full, we overwrote the oldest data at _head (wait, _head points to next empty slot).
        // Actually: if full, head overwrites what IS at head.
        // Standard Ring Buffer: Head = Write, Tail = Read.
        // If full, head == tail.
        // We wrote to _head (old position). Then moved _head.
        // If full, now _head == _tail? 
        // Let's count-based impl.
        _tail = (_tail + 1) % TELEMETRY_QUEUE_SIZE; // Drop oldest
    } else {
        _count++;
    }
    return true; 
}

bool TelemetryQueueDriver::pop(TelemetryRecord& record) {
    if (_count == 0) return false;

    record = _buffer[_tail];
    _tail = (_tail + 1) % TELEMETRY_QUEUE_SIZE;
    _count--;
    return true;
}

bool TelemetryQueueDriver::peek(TelemetryRecord& record) {
    if (_count == 0) return false;
    record = _buffer[_tail];
    return true;
}

bool TelemetryQueueDriver::isEmpty() const {
    return _count == 0;
}

bool TelemetryQueueDriver::isFull() const {
    return _count == TELEMETRY_QUEUE_SIZE;
}

uint16_t TelemetryQueueDriver::count() const {
    return _count;
}

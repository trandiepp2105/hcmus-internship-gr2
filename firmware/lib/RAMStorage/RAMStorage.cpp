#include "RAMStorage.h"

RAMStorage::RAMStorage() : head(0), tail(0), count(0) {}

bool RAMStorage::push(TelemetryData data) {
    // Lưu struct trực tiếp vào mảng tại vị trí head
    buffer[head] = data; 
    
    // Di chuyển chỉ số head theo vòng tròn
    head = (head + 1) % MAX_ITEMS;

    if (count < MAX_ITEMS) {
        count++; // Tăng số lượng nếu chưa đầy
    } else {
        // Nếu đầy, ghi đè bản ghi cũ nhất và di chuyển tail
        tail = (tail + 1) % MAX_ITEMS;
    }
    return true;
}

bool RAMStorage::pop(TelemetryData &output) {
    if (count == 0) return false;

    // Lấy dữ liệu từ vị trí tail (FIFO)
    output = buffer[tail];
    tail = (tail + 1) % MAX_ITEMS;
    count--;
    return true;
}

int RAMStorage::getCount() { return count; }
bool RAMStorage::isEmpty() { return count == 0; }
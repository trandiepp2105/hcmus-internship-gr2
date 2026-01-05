#include "ram_buffer.h"
#include <string.h>

// Khởi tạo các giá trị ban đầu cho bộ đệm
RAMBuffer::RAMBuffer() : head(0), tail(0), count(0)
{
    // Xóa sạch vùng nhớ mảng để đảm bảo không có dữ liệu rác
    memset(buffer, 0, sizeof(buffer));
}

// Hàm đẩy dữ liệu vào bộ đệm
bool RAMBuffer::push(const char *data)
{
    // Copy chuỗi dữ liệu vào vị trí 'head'
    strncpy(buffer[head], data, ITEM_SIZE - 1);
    buffer[head][ITEM_SIZE - 1] = '\0'; // Đảm bảo luôn có ký tự kết thúc chuỗi

    // Di chuyển chỉ số 'head' sang vị trí tiếp theo theo vòng tròn
    head = (head + 1) % MAX_ITEMS;

    if (count < MAX_ITEMS)
    {
        // Nếu chưa đầy thì tăng số lượng bản ghi
        count++;
    }
    else
    {
        // Nếu đã đầy, vị trí 'tail' (cũ nhất) bị đẩy đi để nhường chỗ
        tail = (tail + 1) % MAX_ITEMS;
    }
    return true;
}

// Hàm lấy dữ liệu cũ nhất ra (First In First Out - FIFO)
bool RAMBuffer::pop(char *output)
{
    if (count == 0)
        return false; // Không có dữ liệu để lấy

    // Copy dữ liệu từ vị trí 'tail' ra biến đầu ra
    strncpy(output, buffer[tail], ITEM_SIZE);

    // Di chuyển chỉ số 'tail' sang vị trí tiếp theo theo vòng tròn
    tail = (tail + 1) % MAX_ITEMS;

    // Giảm số lượng bản ghi hiện có
    count--;
    return true;
}

int RAMBuffer::getCount() { return count; }
bool RAMBuffer::isEmpty() { return count == 0; }
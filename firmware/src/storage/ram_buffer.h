#ifndef RAM_BUFFER_H
#define RAM_BUFFER_H

#include <Arduino.h>

// Định nghĩa kích thước bộ đệm
#define MAX_ITEMS 800 // Lưu tối đa 800 bản ghi
#define ITEM_SIZE 150 // Mỗi bản ghi tối đa 150 ký tự (đủ cho chuỗi JSON)

class RAMBuffer
{
private:
    // Mảng 2 chiều lưu trữ dữ liệu trực tiếp trên SRAM
    char buffer[MAX_ITEMS][ITEM_SIZE];
    int head;  // Chỉ số vị trí sẽ ghi dữ liệu mới vào
    int tail;  // Chỉ số vị trí sẽ lấy dữ liệu cũ ra
    int count; // Số lượng bản ghi hiện đang có trong bộ đệm

public:
    // Hàm khởi tạo bộ đệm
    RAMBuffer();

    // Thêm dữ liệu vào bộ đệm (khi mất mạng)
    bool push(const char *data);

    // Lấy dữ liệu cũ nhất ra (để gửi bù khi có mạng)
    bool pop(char *output);

    // Kiểm tra số lượng bản ghi hiện tại
    int getCount();

    // Kiểm tra bộ đệm trống hay không
    bool isEmpty();
};

#endif
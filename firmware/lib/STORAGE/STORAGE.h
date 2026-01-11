#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

/**
 * @brief Kích thước bộ đệm vòng để đảm bảo an toàn DRAM cho ESP32.
 * @details Giới hạn 500 bản ghi để tránh lỗi overflow dram0_0_seg.
 */
#define MAX_ITEMS 500
#define ITEM_SIZE 150

/**
 * @class STORAGE
 * @brief Lớp Driver quản lý việc lưu trữ dữ liệu thô vào RAM (SRAM).
 */
class STORAGE
{
private:
    char buffer[MAX_ITEMS][ITEM_SIZE];
    int head;
    int tail;
    int count;

public:
    /** @brief Khởi tạo vùng nhớ đệm vòng trên RAM */
    STORAGE();

    /**
     * @brief Đẩy một bản ghi dữ liệu vào bộ đệm.
     * @param data Chuỗi dữ liệu cần lưu (thường là JSON).
     * @return true nếu lưu thành công.
     */
    bool push(const char *data);

    /**
     * @brief Lấy bản ghi cũ nhất từ bộ đệm ra (FIFO).
     * @param output Con trỏ chứa dữ liệu lấy ra.
     * @return true nếu lấy được dữ liệu, false nếu bộ đệm trống.
     */
    bool pop(char *output);

    /**
     * @brief Đọc giá trị trạng thái từ bộ đệm.
     * @param key Tên khóa cần kiểm tra (ví dụ: "is_empty").
     * @param defaultValue Giá trị mặc định trả về.
     * @return true/false tương ứng với trạng thái.
     */
    bool readBool(const char *key, bool defaultValue);

    int getCount();
    bool isEmpty();
};

#endif
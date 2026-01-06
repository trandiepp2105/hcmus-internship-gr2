#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "ButtonDriver.h"

/**
 * @class ButtonHandler
 * @brief Lớp xử lý sự kiện nút nhấn mức cao hơn dựa trên ButtonDriver.
 *
 * Lớp này cung cấp phương thức kiểm tra sự kiện "nhấn và thả" (clicked)
 * dựa trên trạng thái đã được chống nhiễu từ ButtonDriver.
 */
class ButtonHandler {
public:
    /**
     * @brief Khởi tạo đối tượng ButtonHandler với một ButtonDriver cụ thể.
     * @param driver Con trỏ tới đối tượng ButtonDriver đã được khởi tạo.
     */
    ButtonHandler(ButtonDriver* driver);

    /**
     * @brief Kiểm tra xem nút đã được nhấn và thả (clicked) hay chưa.
     *
     * Hàm này trả về true duy nhất một lần cho mỗi lần nhấn-thả nút.
     * Nên được gọi thường xuyên trong vòng lặp chính.
     * @return true nếu vừa phát hiện sự kiện click, false nếu không.
     */
    bool checkClicked();

private:
    ButtonDriver* _driver; ///< Con trỏ tới đối tượng ButtonDriver để lấy trạng thái nút.
    bool _wasPressed;      ///< Lưu trạng thái nút ở lần kiểm tra trước để phát hiện cạnh.
};
#endif
/**
 * @file Storage.h
 * @brief Storage Driver - Cung cấp API để lưu trữ dữ liệu key-value vào bộ nhớ flash
 * 
 * @description 
 * Driver này sử dụng thư viện Preferences của ESP32 để lưu trữ dữ liệu
 * bền vững (persistent) vào NVS (Non-Volatile Storage). Dữ liệu được
 * giữ lại ngay cả khi mất điện hoặc reset ESP32.
 * 
 * @note Chỉ hỗ trợ ESP32 family (ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6)
 * 
 * @usage
 * ============================================================================
 * HƯỚNG DẪN SỬ DỤNG
 * ============================================================================
 * 
 * 1. Include header:
 *    #include <Storage.h>
 * 
 * 2. Khởi tạo đối tượng:
 *    Storage storage;
 * 
 * 3. Mở storage với namespace (tối đa 15 ký tự):
 *    storage.begin("my-config");
 * 
 * 4. Đọc/ghi dữ liệu:
 *    // Ghi
 *    storage.putInt("boot_count", 42);
 *    storage.putFloat("ph_offset", 0.15f);
 *    storage.putBool("is_calibrated", true);
 *    storage.putString("wifi_ssid", "MyNetwork");
 * 
 *    // Đọc (tham số thứ 2 là giá trị mặc định nếu key không tồn tại)
 *    int count = storage.getInt("boot_count", 0);
 *    float offset = storage.getFloat("ph_offset", 0.0f);
 *    bool calibrated = storage.getBool("is_calibrated", false);
 *    String ssid = storage.getString("wifi_ssid", "");
 * 
 * 5. Các thao tác khác:
 *    storage.isKeyExist("my_key");  // Kiểm tra key tồn tại
 *    storage.remove("my_key");       // Xóa một key
 *    storage.clear();                // Xóa toàn bộ namespace
 *    storage.freeEntries();          // Số entry còn trống
 * 
 * 6. Đóng storage khi không dùng nữa:
 *    storage.end();
 * 
 * ============================================================================
 * VÍ DỤ HOÀN CHỈNH
 * ============================================================================
 * 
 * #include <Storage.h>
 * 
 * Storage storage;
 * 
 * void setup() {
 *     Serial.begin(115200);
 *     
 *     // Khởi tạo storage
 *     if (!storage.begin("ph-config")) {
 *         Serial.println("Failed to init storage!");
 *         return;
 *     }
 *     
 *     // Lưu cấu hình calibration
 *     storage.putFloat("ph_offset", 0.15f);
 *     storage.putFloat("ph_slope", 1.02f);
 *     
 *     // Đọc lại
 *     float offset = storage.getFloat("ph_offset", 0.0f);
 *     Serial.printf("pH Offset: %.4f\n", offset);
 *     
 *     storage.end();
 * }
 * 
 * ============================================================================
 * LƯU Ý
 * ============================================================================
 * - Key tối đa 15 ký tự
 * - Namespace tối đa 15 ký tự
 * - Phải gọi begin() trước khi đọc/ghi
 * - Nên gọi end() khi không sử dụng nữa
 * - Dữ liệu được lưu vào flash, hạn chế ghi quá nhiều lần
 * 
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <Preferences.h>

class Storage {
public:
    /**
     * @brief Constructor - Khởi tạo đối tượng Storage
     */
    Storage();

    /**
     * @brief Destructor - Giải phóng tài nguyên
     */
    ~Storage();

    /**
     * @brief Khởi tạo storage với namespace được chỉ định
     * @param namespaceName Tên namespace để phân chia dữ liệu (tối đa 15 ký tự)
     * @param readOnly true = chỉ đọc, false = đọc/ghi
     * @return true nếu khởi tạo thành công, false nếu thất bại
     */
    bool begin(const char* namespaceName, bool readOnly = false);

    /**
     * @brief Đóng storage và giải phóng tài nguyên
     */
    void end();

    // ==================== Integer Operations ====================

    /**
     * @brief Lưu giá trị integer vào storage
     * @param key Tên key (tối đa 15 ký tự)
     * @param value Giá trị cần lưu
     * @return true nếu lưu thành công, false nếu thất bại
     */
    bool putInt(const char* key, int32_t value);

    /**
     * @brief Đọc giá trị integer từ storage
     * @param key Tên key cần đọc
     * @param defaultValue Giá trị mặc định nếu key không tồn tại
     * @return Giá trị đọc được hoặc defaultValue nếu key không tồn tại
     */
    int32_t getInt(const char* key, int32_t defaultValue = 0);

    // ==================== Float Operations ====================

    /**
     * @brief Lưu giá trị float vào storage
     * @param key Tên key (tối đa 15 ký tự)
     * @param value Giá trị cần lưu
     * @return true nếu lưu thành công, false nếu thất bại
     */
    bool putFloat(const char* key, float value);

    /**
     * @brief Đọc giá trị float từ storage
     * @param key Tên key cần đọc
     * @param defaultValue Giá trị mặc định nếu key không tồn tại
     * @return Giá trị đọc được hoặc defaultValue nếu key không tồn tại
     */
    float getFloat(const char* key, float defaultValue = 0.0f);

    // ==================== Double Operations ====================

    /**
     * @brief Lưu giá trị double vào storage
     * @param key Tên key (tối đa 15 ký tự)
     * @param value Giá trị cần lưu
     * @return true nếu lưu thành công, false nếu thất bại
     */
    bool putDouble(const char* key, double value);

    /**
     * @brief Đọc giá trị double từ storage
     * @param key Tên key cần đọc
     * @param defaultValue Giá trị mặc định nếu key không tồn tại
     * @return Giá trị đọc được hoặc defaultValue nếu key không tồn tại
     */
    double getDouble(const char* key, double defaultValue = 0.0);

    // ==================== Boolean Operations ====================

    /**
     * @brief Lưu giá trị boolean vào storage
     * @param key Tên key (tối đa 15 ký tự)
     * @param value Giá trị cần lưu
     * @return true nếu lưu thành công, false nếu thất bại
     */
    bool putBool(const char* key, bool value);

    /**
     * @brief Đọc giá trị boolean từ storage
     * @param key Tên key cần đọc
     * @param defaultValue Giá trị mặc định nếu key không tồn tại
     * @return Giá trị đọc được hoặc defaultValue nếu key không tồn tại
     */
    bool getBool(const char* key, bool defaultValue = false);

    // ==================== String Operations ====================

    /**
     * @brief Lưu chuỗi String vào storage
     * @param key Tên key (tối đa 15 ký tự)
     * @param value Chuỗi cần lưu
     * @return true nếu lưu thành công, false nếu thất bại
     */
    bool putString(const char* key, const String& value);

    /**
     * @brief Đọc chuỗi String từ storage
     * @param key Tên key cần đọc
     * @param defaultValue Giá trị mặc định nếu key không tồn tại
     * @return Chuỗi đọc được hoặc defaultValue nếu key không tồn tại
     */
    String getString(const char* key, const String& defaultValue = "");

    // ==================== Utility Operations ====================

    /**
     * @brief Kiểm tra key có tồn tại trong storage không
     * @param key Tên key cần kiểm tra
     * @return true nếu key tồn tại, false nếu không
     */
    bool isKeyExist(const char* key);

    /**
     * @brief Xóa một key khỏi storage
     * @param key Tên key cần xóa
     * @return true nếu xóa thành công, false nếu thất bại
     */
    bool remove(const char* key);

    /**
     * @brief Xóa toàn bộ dữ liệu trong namespace hiện tại
     * @return true nếu xóa thành công, false nếu thất bại
     */
    bool clear();

    /**
     * @brief Lấy số byte còn trống trong NVS
     * @return Số byte còn trống
     */
    size_t freeEntries();

private:
    Preferences _preferences;  // Đối tượng Preferences của ESP32
    bool _initialized;         // Trạng thái khởi tạo

    /**
     * @brief Kiểm tra xem storage đã được khởi tạo chưa
     * @return true nếu đã khởi tạo, false nếu chưa
     */
    bool isInitialized();
};

#endif // STORAGE_H

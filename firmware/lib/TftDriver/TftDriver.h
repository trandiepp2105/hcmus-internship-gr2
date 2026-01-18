#ifndef TFT_DRIVER_H
#define TFT_DRIVER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

/**
 * @class TftDriver
 * @brief Lop dieu khien man hinh TFT LCD ST7735 qua giao tiep SPI.
 * 
 * Driver nay wrap thu vien Adafruit_ST7735 va cung cap day du cac chuc nang
 * hien thi text, ve hinh, va hien thi hinh anh.
 */
class TftDriver {
public:
    /**
     * @brief Khoi tao TftDriver voi Hardware SPI (chi can CS, DC, RST).
     * MOSI va SCK su dung chan mac dinh cua ESP32 (GPIO 23, 18).
     */
    TftDriver(int8_t cs, int8_t dc, int8_t rst = -1);
    
    /**
     * @brief Khoi tao TftDriver voi Software SPI (dinh nghia tat ca cac chan).
     * @param cs Chan Chip Select.
     * @param dc Chan Data/Command.
     * @param mosi Chan MOSI (SDA).
     * @param sck Chan SCK (Clock).
     * @param rst Chan Reset (-1 neu khong su dung).
     */
    TftDriver(int8_t cs, int8_t dc, int8_t mosi, int8_t sck, int8_t rst = -1);

    // ==================== KHOI TAO VA DIEU KHIEN ====================

    /**
     * @brief Khoi tao man hinh ST7735.
     * @param tabColor Loai man hinh: INITR_BLACKTAB, INITR_GREENTAB, INITR_REDTAB, etc.
     */
    void init(uint8_t tabColor = INITR_BLACKTAB);

    /**
     * @brief Xoay huong man hinh.
     * @param rotation Gia tri 0-3 (0=0°, 1=90°, 2=180°, 3=270°).
     */
    void setRotation(uint8_t rotation);

    /**
     * @brief Dao nguoc mau hien thi.
     * @param invert true de dao nguoc, false de binh thuong.
     */
    void invertDisplay(bool invert);

    /**
     * @brief Lay chieu rong man hinh (pixels).
     * @return Chieu rong theo rotation hien tai.
     */
    int16_t width();

    /**
     * @brief Lay chieu cao man hinh (pixels).
     * @return Chieu cao theo rotation hien tai.
     */
    int16_t height();

    // ==================== XOA VA TO MAU ====================

    /**
     * @brief Xoa toan bo man hinh voi mau den.
     */
    void clear();

    /**
     * @brief To toan bo man hinh voi mau chi dinh.
     * @param color Ma mau 16-bit (RGB565).
     */
    void fillScreen(uint16_t color);

    // ==================== VE DIEM VA DUONG ====================

    /**
     * @brief Ve mot diem anh.
     * @param x Toa do X.
     * @param y Toa do Y.
     * @param color Ma mau.
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color);

    /**
     * @brief Ve duong thang giua hai diem.
     * @param x0, y0 Diem bat dau.
     * @param x1, y1 Diem ket thuc.
     * @param color Ma mau.
     */
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

    /**
     * @brief Ve duong ngang nhanh.
     * @param x Toa do X bat dau.
     * @param y Toa do Y.
     * @param w Chieu dai.
     * @param color Ma mau.
     */
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

    /**
     * @brief Ve duong doc nhanh.
     * @param x Toa do X.
     * @param y Toa do Y bat dau.
     * @param h Chieu cao.
     * @param color Ma mau.
     */
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

    // ==================== HINH CHU NHAT ====================

    /**
     * @brief Ve khung hinh chu nhat.
     */
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    /**
     * @brief Ve hinh chu nhat dac.
     */
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    /**
     * @brief Ve khung hinh chu nhat bo goc.
     * @param r Ban kinh bo goc.
     */
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

    /**
     * @brief Ve hinh chu nhat dac bo goc.
     */
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

    // ==================== HINH TRON ====================

    /**
     * @brief Ve duong tron.
     * @param x0, y0 Tam.
     * @param r Ban kinh.
     * @param color Ma mau.
     */
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

    /**
     * @brief Ve hinh tron dac.
     */
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

    // ==================== TAM GIAC ====================

    /**
     * @brief Ve khung tam giac.
     */
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
                      int16_t x2, int16_t y2, uint16_t color);

    /**
     * @brief Ve tam giac dac.
     */
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
                      int16_t x2, int16_t y2, uint16_t color);

    // ==================== TEXT ====================

    /**
     * @brief Dat vi tri con tro de in text.
     * @param x Toa do X.
     * @param y Toa do Y.
     */
    void setCursor(int16_t x, int16_t y);

    /**
     * @brief Dat mau chu.
     * @param color Mau chu.
     */
    void setTextColor(uint16_t color);

    /**
     * @brief Dat mau chu va mau nen.
     * @param color Mau chu.
     * @param bg Mau nen.
     */
    void setTextColor(uint16_t color, uint16_t bg);

    /**
     * @brief Dat kich thuoc chu.
     * @param size He so nhan (1 = 6x8 pixels, 2 = 12x16 pixels, etc.).
     */
    void setTextSize(uint8_t size);

    /**
     * @brief Bat/tat tu dong xuong dong.
     * @param wrap true de tu dong xuong dong.
     */
    void setTextWrap(bool wrap);

    /**
     * @brief In text tai vi tri hien tai cua con tro.
     * @param text Chuoi can in.
     */
    void print(const String& text);

    /**
     * @brief In text va xuong dong.
     * @param text Chuoi can in.
     */
    void println(const String& text);

    /**
     * @brief In so nguyen.
     * @param value Gia tri can in.
     */
    void print(int value);

    /**
     * @brief In so thuc.
     * @param value Gia tri can in.
     * @param decimals So chu so thap phan.
     */
    void print(float value, int decimals = 2);

    /**
     * @brief In text tai vi tri chi dinh (ham tien ich).
     * @param x Toa do X.
     * @param y Toa do Y.
     * @param text Noi dung can in.
     * @param color Mau chu.
     * @param size Kich thuoc chu.
     */
    void printAt(int16_t x, int16_t y, const String& text, 
                 uint16_t color = ST77XX_WHITE, uint8_t size = 1);

    // ==================== BITMAP/HINH ANH ====================

    /**
     * @brief Ve bitmap don sac.
     * @param x, y Vi tri.
     * @param bitmap Con tro den mang bitmap.
     * @param w, h Kich thuoc.
     * @param color Mau ve.
     */
    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                    int16_t w, int16_t h, uint16_t color);

    /**
     * @brief Ve bitmap don sac voi mau nen.
     */
    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                    int16_t w, int16_t h, uint16_t color, uint16_t bg);

    /**
     * @brief Ve bitmap RGB565.
     * @param bitmap Con tro den mang RGB565.
     */
    void drawRGBBitmap(int16_t x, int16_t y, const uint16_t* bitmap,
                       int16_t w, int16_t h);

    // ==================== TIEN ICH MAU ====================

    /**
     * @brief Chuyen doi mau RGB sang RGB565.
     * @param r Gia tri do (0-255).
     * @param g Gia tri xanh la (0-255).
     * @param b Gia tri xanh duong (0-255).
     * @return Ma mau 16-bit RGB565.
     */
    static uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Truy cap doi tuong Adafruit_ST7735 goc.
     * @return Tham chieu den doi tuong TFT.
     */
    Adafruit_ST7735& getTft();

private:
    Adafruit_ST7735 _tft;
};

// ==================== DINH NGHIA MAU CO BAN ====================
// Su dung cac hang so tu Adafruit_ST77xx:
// ST77XX_BLACK, ST77XX_WHITE, ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE
// ST77XX_CYAN, ST77XX_MAGENTA, ST77XX_YELLOW, ST77XX_ORANGE

#endif

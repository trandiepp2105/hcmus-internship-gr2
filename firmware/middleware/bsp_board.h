#ifndef BSP_BOARD_H
#define BSP_BOARD_H

#include <Arduino.h>

// I2C Configuration
#define LCD_ADDR        0x27
#define LCD_COLS        16
#define LCD_ROWS        2

// WiFi Configuration
#define WIFI_RECONNECT_TIMEOUT_MS  60000  // 60 seconds before opening portal

#ifdef ESP32
    // --- ESP32 Configuration ---
    #define PIN_I2C_SDA     21
    #define PIN_I2C_SCL     22
    
    // Potentiometers (ADC Input)
    #define PIN_POT_UPPER   39  // Analog pin for Upper Threshold
    #define PIN_POT_LOWER   35  // Analog pin for Lower Threshold
    
    // Buttons  
    #define PIN_BTN_A       16   // Mode Button
    #define PIN_BTN_B       17   // Action/Select Button
    #define PIN_BTN_C       19   // Calibration Config Button
    
    // Temperature Sensor
    #define PIN_TEMP_SENSOR 15  // DS18B20 Data Pin
    
    // 74HC595 Shift Register (Relay Control)
    #define PIN_SR_DATA     33  // DS (Serial Data)
    #define PIN_SR_CLOCK    25  // SHCP (Shift Clock)
    #define PIN_SR_LATCH    32  // STCP (Latch)
    
    // TFT ST7735 Display (SPI)
    #define PIN_TFT_CS      5   // Chip Select
    #define PIN_TFT_DC      4   // Data/Command
    #define PIN_TFT_RST     2   // Reset
    #define PIN_TFT_MOSI    23  // SPI MOSI (SDA)
    #define PIN_TFT_SCK     18  // SPI Clock (SCK)

#elif defined(ESP8266)
    // --- ESP8266 Configuration ---
    #define PIN_I2C_SDA     4   // D2
    #define PIN_I2C_SCL     5   // D1
    
    // Potentiometers (ADC Input)
    // Note: ESP8266 only has ONE Analog Pin (A0). 
    // Both mapped to A0 for now unless external Mux is used.
    #define PIN_POT_UPPER   A0 
    #define PIN_POT_LOWER   A0  
    
    // Buttons
    #define PIN_BTN_A       0   // D3 (Flash) - Check manual pullup
    #define PIN_BTN_B       14  // D5 (Use D5/GPIO14 instead of GPIO5 which is SCL)
    // Note: GPIO5 on 8266 is usually D1 (SCL). GPIO4 is D2 (SDA).
    // Button B was 5 (SCL conflict). Changing B to 14 (D5) or 12 (D6).
    // Let's use 14 (D5).
    #define PIN_TEMP_SENSOR 12  // D6 (GPIO12) for DS18B20
    
    // 74HC595 Shift Register (Relay Control)
    #define PIN_SR_DATA     13  // D7 (GPIO13)
    #define PIN_SR_CLOCK    15  // D8 (GPIO15)
    #define PIN_SR_LATCH    16  // D0 (GPIO16)
#else
    #error "Unsupported Board! Please use ESP32 or ESP8266."
#endif

// Helper function to initialize board components
void BSP_Init();

#endif // BSP_BOARD_H

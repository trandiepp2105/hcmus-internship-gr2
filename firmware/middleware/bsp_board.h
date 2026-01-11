#ifndef BSP_BOARD_H
#define BSP_BOARD_H

#include <Arduino.h>

// I2C Configuration
#define LCD_ADDR        0x27
#define LCD_COLS        16
#define LCD_ROWS        2
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22

// Buttons
#define PIN_BTN_A       0   // Mode Button (was SWITCH)
#define PIN_BTN_B       5   // Action/Select Button (was CALIB)

// Potentiometers (ADC Input)
#define PIN_POT_UPPER   34  // Analog pin for Upper Threshold
#define PIN_POT_LOWER   35  // Analog pin for Lower Threshold

// Helper function to initialize board components
void BSP_Init();

#endif // BSP_BOARD_H
#ifndef BSP_BOARD_H
#define BSP_BOARD_H

//LCD configuration
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// Button pin
#define PIN_BTN_SWITCH 0
#define PIN_BTN_CALIB  5

// Potentiometer pin
#define PIN_POT_UPPER  A0
#define PIN_POT_LOWER  A0


// helper khởi tạo các thành phần của board
void BSP_Init();

#endif
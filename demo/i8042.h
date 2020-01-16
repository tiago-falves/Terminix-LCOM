 #pragma once
  
 #include <stdbool.h>
 #include <stdint.h>


  
 #define STAT_REG 0x64
 #define KBC_COMMAND_REG 0x64
 #define KBC_ARGUMENT_REG 0x60
 #define WRITE_COMMAND_BYTE 0x60
 #define READ_COMMAND_BYTE 0x20
 #define OUT_BUF_REG 0x60
 #define INPUT_BUF_REG 0x60
 #define OBF_BIT BIT(0)
 #define INT0 BIT(0)
 #define PARITY_BIT BIT(7)
 #define TIME_OUT_BIT BIT(6)
 #define AUX_BIT BIT(5)
 #define DELAY_US 15
 #define INCOMPLETE_SCAN_CODE 0xE0
 #define IBF_BIT BIT(1)
 #define ESC_BREAK_CODE 0x81
 #define W_MAKE_CODE 0x11
 #define W_BREAK_CODE 0x91
 #define A_MAKE_CODE 0x1E
 #define A_BREAK_CODE 0x9E
 #define D_MAKE_CODE 0x20
 #define D_BREAK_CODE 0xA0
 #define S_MAKE_CODE 0x1F
 #define S_BREAK_CODE 0x9F

 //Mouse Related Macros
 #define MOUSE_DIS_DATA_REPORTING 0xF5
#define MOUSE_EN_DATA_REPORTING 0xF4
#define MOUSE_READ_DATA 0xEB
#define MOUSE_SET_STREAM_MODE 0xEA
#define ACK 0xFA
#define WRITE_BYTE_TO_MOUSE 0xD4
#define MOUSE_SET_REMOTE_MODE 0xF0
#define MSB_X_DELTA_BIT BIT(4)
#define MSB_Y_DELTA_BIT BIT(5)
#define EXTEND_8_TO_16 0XFF00



 int keyboard_subscribe(uint8_t *bit_no);

 int keyboard_unsubscribe();

 int reenable_keyboard_interrupt();

 int flush_OBF();

 int got_full_scan_code();

 void print_full_scan_code();

 int run_until_ESC_key();

 int keyboard_handler();

 


// hid/hid_usage.h
#ifndef HID_USAGE_H
#define HID_USAGE_H

#include <stdint.h>

// 键盘修饰键定义
#define KEY_MOD_LCTRL  0x01  // 左Ctrl
#define KEY_MOD_LSHIFT 0x02  // 左Shift
#define KEY_MOD_LALT   0x04  // 左Alt
#define KEY_MOD_LGUI   0x08  // 左GUI（通常是Windows键）
#define KEY_MOD_RCTRL  0x10  // 右Ctrl
#define KEY_MOD_RSHIFT 0x20  // 右Shift
#define KEY_MOD_RALT   0x40  // 右Alt
#define KEY_MOD_RGUI   0x80  // 右GUI

// 常用按键扫描码定义（字母）
#define KEY_A 0x04
#define KEY_B 0x05
#define KEY_C 0x06
#define KEY_D 0x07
#define KEY_E 0x08
#define KEY_F 0x09
#define KEY_G 0x0A
#define KEY_H 0x0B
#define KEY_I 0x0C
#define KEY_J 0x0D
#define KEY_K 0x0E
#define KEY_L 0x0F
#define KEY_M 0x10
#define KEY_N 0x11
#define KEY_O 0x12
#define KEY_P 0x13
#define KEY_Q 0x14
#define KEY_R 0x15
#define KEY_S 0x16
#define KEY_T 0x17
#define KEY_U 0x18
#define KEY_V 0x19
#define KEY_W 0x1A
#define KEY_X 0x1B
#define KEY_Y 0x1C
#define KEY_Z 0x1D

// 常用按键扫描码定义（数字）
#define KEY_1 0x1E
#define KEY_2 0x1F
#define KEY_3 0x20
#define KEY_4 0x21
#define KEY_5 0x22
#define KEY_6 0x23
#define KEY_7 0x24
#define KEY_8 0x25
#define KEY_9 0x26
#define KEY_0 0x27

// 常用按键扫描码定义（特殊键）
#define KEY_ENTER      0x28    // 回车键
#define KEY_ESC        0x29    // ESC键
#define KEY_BACKSPACE  0x2A    // 退格键
#define KEY_TAB        0x2B    // Tab键
#define KEY_SPACE      0x2C    // 空格键
#define KEY_MINUS      0x2D    // 减号键
#define KEY_EQUAL      0x2E    // 等号键
#define KEY_LEFTBRACE  0x2F    // 左方括号键
#define KEY_RIGHTBRACE 0x30    // 右方括号键
#define KEY_BACKSLASH  0x31    // 反斜杠键
#define KEY_SEMICOLON  0x33    // 分号键
#define KEY_APOSTROPHE 0x34    // 单引号键
#define KEY_GRAVE      0x35    // 反引号键
#define KEY_COMMA      0x36    // 逗号键
#define KEY_DOT        0x37    // 句号键
#define KEY_SLASH      0x38    // 斜杠键

// 功能键扫描码定义
#define KEY_F1  0x3A
#define KEY_F2  0x3B
#define KEY_F3  0x3C
#define KEY_F4  0x3D
#define KEY_F5  0x3E
#define KEY_F6  0x3F
#define KEY_F7  0x40
#define KEY_F8  0x41
#define KEY_F9  0x42
#define KEY_F10 0x43
#define KEY_F11 0x44
#define KEY_F12 0x45

// 方向键扫描码定义
#define KEY_UP    0x52    // 上方向键
#define KEY_DOWN  0x51    // 下方向键
#define KEY_LEFT  0x50    // 左方向键
#define KEY_RIGHT 0x4F    // 右方向键

// Android系统按键 Consumer Control 用法码定义
#define CONSUMER_BACK        0x0224    // Android返回键 (AC Back)
#define CONSUMER_HOME        0x0223    // Android Home键 (AC Home)
#define CONSUMER_MENU        0x0194    // Android通知面板键 (AL Local Machine Browser)
#define CONSUMER_APP_SWITCH  0x01A2    // Android最近任务键 (AC Desktop Show All Applications)


void send_keyobard_report(uint8_t modifier, uint8_t key1, uint8_t key2,uint8_t key3);
void send_touch_report(uint8_t state, int16_t x, int16_t y);

#endif // HID_USAGE_H
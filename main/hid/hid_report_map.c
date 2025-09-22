#include "esp_hid_common.h"

const unsigned char hid_report_map[] = {
    // 触摸屏报告 (Report ID 1) - 用于绝对定位（触控逻辑）
    0x05, 0x0d,                    // USAGE_PAGE (Digitizer)
    0x09, 0x04,                    // USAGE (Touch Screen)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,                    //   REPORT_ID (1)

    // 触摸点集合
    0x09, 0x20,                    //   Usage (Stylus)
    0xA1, 0x00,                    //   Collection (Physical)

    // 触摸状态 (触摸按下/抬起)
    0x09, 0x42,                    //     Usage (Tip Switch)
    0x09, 0x32,                    //     USAGE (In Range)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)

    // 剩余6位填充
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x95, 0x06,                    //     REPORT_COUNT (6)
    0x81, 0x01,                    //     INPUT (Cnst,Ary,Abs)

    // 绝对X和Y坐标 (16位)
    0x05, 0x01,                    //     Usage Page (Generic Desktop)
    0x09, 0x01,                    //     Usage (Pointer)
    0xA1, 0x00,                    //     Collection (Physical)
    0x09, 0x30,                    //        Usage (X)
    0x09, 0x31,                    //        Usage (Y)
    0x16, 0x00, 0x00,              //        Logical Minimum (0)
    0x26, 0x10, 0x27,              //        Logical Maximum (10000)
    0x36, 0x00, 0x00,              //        Physical Minimum (0)
    0x46, 0x10, 0x27,              //        Physical Maximum (10000)
    0x66, 0x00, 0x00,              //        UNIT (None)
    0x75, 0x10,                    //        Report Size (16)
    0x95, 0x02,                    //        Report Count (2)
    0x81, 0x02,                    //        Input (Data,Var,Abs)
    0xc0,                          //     END_COLLECTION
    0xc0,                          //   END_COLLECTION
    0xc0,                          // END_COLLECTION

    // 键盘报告 (Report ID 3)
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xA1, 0x01, // COLLECTION (Application)
    0x85, 0x03, //   REPORT_ID (3)

    // 修饰键 (Ctrl, Shift, Alt, GUI等)
    0x05, 0x07, //   USAGE_PAGE (Keyboard/Keypad)
    0x19, 0xE0, //   USAGE_MINIMUM (Left Control)
    0x29, 0xE7, //   USAGE_MAXIMUM (Right GUI)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0x01, //   LOGICAL_MAXIMUM (1)
    0x75, 0x01, //   REPORT_SIZE (1)
    0x95, 0x08, //   REPORT_COUNT (8)
    0x81, 0x02, //   INPUT (Data,Var,Abs)

    // 保留字节
    0x95, 0x01, //   REPORT_COUNT (1)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x81, 0x03, //   INPUT (Cnst,Var,Abs)

    // 按键数组 (最多6个同时按下的按键)
    0x95, 0x06, //   REPORT_COUNT (6)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0x65, //   LOGICAL_MAXIMUM (101)
    0x05, 0x07, //   USAGE_PAGE (Keyboard/Keypad)
    0x19, 0x00, //   USAGE_MINIMUM (0)
    0x29, 0x65, //   USAGE_MAXIMUM (101)
    0x81, 0x00, //   INPUT (Data,Ary,Abs)

    0xC0, // END_COLLECTION

    // Consumer Control报告 (Report ID 4) - 用于Android系统按键
    0x05, 0x0C,                    // USAGE_PAGE (Consumer)
    0x09, 0x01,                    // USAGE (Consumer Control)
    0xA1, 0x01,                    // COLLECTION (Application)
    0x85, 0x04,                    //   REPORT_ID (4)
    
    // Android Back键
    0x0A, 0x24, 0x02,              //   USAGE (AC Back - 0x224)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    
    // Android Home键
    0x0A, 0x23, 0x02,              //   USAGE (AC Home - 0x223)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    
    // Android 通知面板键 (替代Menu键)
    0x0A, 0x94, 0x01,              //   USAGE (AL Local Machine Browser - 0x194)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    
    // Android App Switch键
    0x0A, 0xA2, 0x01,              //   USAGE (AC Desktop Show All Applications)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    
    // 填充位 (4位)
    0x75, 0x04,                    //   REPORT_SIZE (4)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    
    0xC0                           // END_COLLECTION
};

esp_hid_raw_report_map_t hid_raw_report_map[] = {
    {.data = hid_report_map, .len = sizeof(hid_report_map)},
};
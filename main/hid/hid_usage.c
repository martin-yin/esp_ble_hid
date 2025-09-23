// hid/hid_usage.c
#include "hid_usage.h"
#include "esp_hidd.h"
#include "hid_param.h"
#include "esp_log.h"
#include <string.h>  // 为 memset 和 strchr

static const char *TAG = "hid_usage";

// 发送 HID 键盘报告
void send_keyboard_report(uint8_t modifier, uint8_t key1, uint8_t key2, uint8_t key3) {
    uint8_t buffer[HID_KEYBOARD_REPORT_SIZE];
    memset(buffer, 0, sizeof(buffer));  // 清零缓冲区
    buffer[0] = modifier;
    buffer[2] = key1;  // buffer[1] 保留
    buffer[3] = key2;
    buffer[4] = key3;

    esp_err_t ret = esp_hidd_dev_input_set(s_ble_hid_param.hid_dev, 0, 3, buffer, sizeof(buffer));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send keyboard report: %d", ret);
        return;
    }
    ESP_LOGD(TAG, "Sent keyboard: mod=0x%02X, keys=[0x%02X,0x%02X,0x%02X]", modifier, key1, key2, key3);
}

// 发送 HID 触摸报告（添加边界检查）
void send_touch_report(uint8_t state, int16_t x, int16_t y) {
    if (x < 0 || x > 32767 || y < 0 || y > 32767) {
        ESP_LOGW(TAG, "Invalid touch coords: x=%d, y=%d", x, y);
        return;
    }
    uint8_t buffer[HID_TOUCH_REPORT_SIZE];
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = (state > 0) ? 0x03 : 0x00;  // tip switch + in range
    buffer[1] = x & 0xFF;
    buffer[2] = (x >> 8) & 0xFF;
    buffer[3] = y & 0xFF;
    buffer[4] = (y >> 8) & 0xFF;

    esp_err_t ret = esp_hidd_dev_input_set(s_ble_hid_param.hid_dev, 0, 1, buffer, sizeof(buffer));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send touch report: %d", ret);
    }
    ESP_LOGD(TAG, "Sent touch: state=0x%02X, x=%d, y=%d", buffer[0], x, y);
}

// 字符需要 Shift 的检查（扩展支持更多符号）
bool char_needs_shift(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c == '!' || c == '@' || c == '#' || c == '$' || c == '%' ||
            c == '^' || c == '&' || c == '*' || c == '(' || c == ')' ||
            c == '_' || c == '+' || c == '{' || c == '}' || c == '|' ||
            c == ':' || c == '"' || c == '<' || c == '>' || c == '?' ||
            c == '~');
}

// 字符到扫描码的映射（扩展支持移位符号）
uint8_t char_to_scancode(char c) {
    if (c >= 'a' && c <= 'z') {
        return KEY_A + (c - 'a');
    }
    if (c >= 'A' && c <= 'Z') {
        return KEY_A + (c - 'A');
    }
    if (c >= '1' && c <= '9') {
        return KEY_1 + (c - '1');
    }
    if (c == '0') return KEY_0;

    switch (c) {
    case ' ' : return KEY_SPACE;
    case '\n': return KEY_ENTER;
    case '\t': return KEY_TAB;
    case '\b': return KEY_BACKSPACE;
    case '-' : case '_' : return KEY_MINUS;
    case '=' : case '+' : return KEY_EQUAL;
    case '[' : case '{' : return KEY_LEFTBRACE;
    case ']' : case '}' : return KEY_RIGHTBRACE;
    case '\\': case '|' : return KEY_BACKSLASH;
    case ';' : case ':' : return KEY_SEMICOLON;
    case '\'': case '"' : return KEY_APOSTROPHE;
    case '`' : case '~' : return KEY_GRAVE;
    case ',' : case '<' : return KEY_COMMA;
    case '.' : case '>' : return KEY_DOT;
    case '/' : case '?' : return KEY_SLASH;
    case '!' : return KEY_1;
    case '@' : return KEY_2;
    case '#' : return KEY_3;
    case '$' : return KEY_4;
    case '%' : return KEY_5;
    case '^' : return KEY_6;
    case '&' : return KEY_7;
    case '*' : return KEY_8;
    case '(' : return KEY_9;
    case ')' : return KEY_0;
    default  : return 0;
    }
}

// 发送字符串作为按键序列
void press_keys(const char *str) {
    if (!str) return;

    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];
        uint8_t scancode = char_to_scancode(c);
        if (scancode != 0) {
            uint8_t modifier = char_needs_shift(c) ? KEY_MOD_LSHIFT : 0;
            send_keyboard_report(modifier, scancode, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(10));  // 短暂延迟
            send_keyboard_report(0, 0, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

// 发送键组合
void press_key_combination(uint8_t modifier, uint8_t key) {
    send_keyboard_report(modifier, key, 0, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    send_keyboard_report(0, 0, 0, 0);
}

// 发送消费者键报告（使用 uint16_t 以支持标准 HID）
void send_consumer_key_report(uint16_t usage_code) {
    uint16_t buffer = 0;
    switch (usage_code) {
        case CONSUMER_BACK:       buffer = 1 << 0; break;
        case CONSUMER_HOME:       buffer = 1 << 1; break;
        case CONSUMER_MENU:       buffer = 1 << 2; break;
        case CONSUMER_APP_SWITCH: buffer = 1 << 3; break;
        default:
            ESP_LOGW(TAG, "Unknown consumer key: 0x%04X", usage_code);
            return;
    }

    uint8_t buf_bytes[HID_CONSUMER_REPORT_SIZE];
    buf_bytes[0] = buffer & 0xFF;
    buf_bytes[1] = (buffer >> 8) & 0xFF;

    esp_err_t ret = esp_hidd_dev_input_set(s_ble_hid_param.hid_dev, 0, 4, buf_bytes, sizeof(buf_bytes));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send consumer report: %d", ret);
    }
    ESP_LOGD(TAG, "Sent consumer key: 0x%04X (buffer: 0x%04X)", usage_code, buffer);

    vTaskDelay(pdMS_TO_TICKS(50));  // 按键持续50ms

    memset(buf_bytes, 0, sizeof(buf_bytes));
    esp_hidd_dev_input_set(s_ble_hid_param.hid_dev, 0, 4, buf_bytes, sizeof(buf_bytes));
}

// 发送 HID 坐标触控屏幕
void touch(uint8_t state, int16_t hid_x, int16_t hid_y) {
    send_touch_report(state, hid_x, hid_y);
    vTaskDelay(pdMS_TO_TICKS(50));
    send_touch_report(0, 0, 0);
}

// 长按触摸
void long_touch(uint8_t state, int16_t hid_x, int16_t hid_y, uint32_t delay_ms) {
    send_touch_report(state, hid_x, hid_y);
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
    send_touch_report(0, 0, 0);
}
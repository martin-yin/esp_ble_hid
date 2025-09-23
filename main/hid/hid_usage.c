// hid/hid_usage.c
#include "hid_usage.h"
#include "esp_hidd.h"
#include "hid_param.h"
#include "esp_log.h"
#include "hid_usage.h"

static const char *TAG = "hid_usage";

// 发送HID报告的通用工具函数
void send_keyobard_report(uint8_t modifier, uint8_t key1, uint8_t key2,
                       uint8_t key3) {
  uint8_t buffer[8] = {0};
  buffer[0] = modifier; // 修饰键
  buffer[1] = 0;        // 保留字节
  buffer[2] = key1;     // 按键1
  buffer[3] = key2;     // 按键2
  buffer[4] = key3;     // 按键3

  esp_hidd_dev_input_set(s_ble_hid_param.hid_dev, 0, 3, buffer, 5);
  ESP_LOGI(TAG,
           "Sent keyboard key: mod=0x%02X, "
           "keys=[0x%02X,0x%02X,0x%02X]",modifier, key1, key2, key3);
}

void send_touch_report(uint8_t state, int16_t x, int16_t y) {
  static uint8_t buffer[5] = {0};

  if (state > 0) {  
    buffer[0] = 0x03;  // 同时设置 tip switch 和 in range（触摸按下）
  } else {
    buffer[0] = 0x00;  // 触摸抬起
  }
  buffer[1] = x & 0xFF;     // X坐标低字节
  buffer[2] = (x >> 8) & 0xFF; // X坐标高字节
  buffer[3] = y & 0xFF;     // Y坐标低字节
  buffer[4] = (y >> 8) & 0xFF; // Y坐标高字节
  
  ESP_LOGI(TAG, "Sending touch: state=0x%02X, x=%d, y=%d", buffer[0], x, y);
  esp_hidd_dev_input_set(s_ble_hid_param.hid_dev, 0, 1, buffer, 5); // 使用Report ID 1
}


bool char_needs_shift(char c) {
  return (c >= 'A' && c <= 'Z') || c == '!' || c == '@' || c == '#' ||
         c == '$' || c == '%' || c == '^' || c == '&' || c == '*' || c == '(' ||
         c == ')' || c == '_' || c == '+' || c == '{' || c == '}' || c == '|' ||
         c == ':' || c == '"' || c == '<' || c == '>' || c == '?';
}
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
  if (c == '0')
    return KEY_0;

  switch (c) {
  case ' ':
    return KEY_SPACE;
  case '\n':
    return KEY_ENTER;
  case '\t':
    return KEY_TAB;
  case '\b':
    return KEY_BACKSPACE;
  case '-':
    return KEY_MINUS;
  case '=':
    return KEY_EQUAL;
  case '[':
    return KEY_LEFTBRACE;
  case ']':
    return KEY_RIGHTBRACE;
  case '\\':
    return KEY_BACKSLASH;
  case ';':
    return KEY_SEMICOLON;
  case '\'':
    return KEY_APOSTROPHE;
  case '`':
    return KEY_GRAVE;
  case ',':
    return KEY_COMMA;
  case '.':
    return KEY_DOT;
  case '/':
    return KEY_SLASH;
  default:
    return 0;
  }
}


void press_keys(const char *str) {
  if (!str)
    return;

  for (int i = 0; str[i] != '\0'; i++) {
    char c = str[i];
    uint8_t scancode = char_to_scancode(c);

    if (scancode != 0) {
      uint8_t modifier = char_needs_shift(c) ? KEY_MOD_LSHIFT : 0;
      send_keyobard_report(modifier, scancode, 0, 0);
      vTaskDelay(pdMS_TO_TICKS(10)); // 短暂延迟
      send_keyobard_report(0, 0, 0, 0); 
      vTaskDelay(pdMS_TO_TICKS(10)); 
    }
  }
}

void press_key_combination(uint8_t modifier, uint8_t key) {
  send_keyobard_report(modifier, key, 0, 0);
  vTaskDelay(pdMS_TO_TICKS(50));
  send_keyobard_report(0, 0, 0, 0); 
}

void send_consumer_key_report(uint16_t usage_code) {
    static uint8_t buffer = 0;
    switch(usage_code) {
        case CONSUMER_BACK:        // 0x0224 - Android返回键
            buffer = 0x01;         // 位0
            break;
        case CONSUMER_HOME:        // 0x0223 - Android Home键
            buffer = 0x02;         // 位1
            break;
        case CONSUMER_MENU:        // 0x0194 - Android通知面板键
            buffer = 0x04;         // 位2
            break;
        case CONSUMER_APP_SWITCH:  // 0x01A2 - Android最近任务键
            buffer = 0x08;         // 位3
            break;
        default:
            ESP_LOGW(TAG, "Unknown consumer key: 0x%04X", usage_code);
            return;
    }
    
    ESP_LOGI(TAG, "Sending consumer key: 0x%04X (buffer: 0x%02X)", usage_code, buffer);
    esp_hidd_dev_input_set(s_ble_hid_param.hid_dev, 0, 4, &buffer, 1);
    vTaskDelay(pdMS_TO_TICKS(50)); // 按键持续50ms
    buffer = 0x00;
    esp_hidd_dev_input_set(s_ble_hid_param.hid_dev, 0, 4, &buffer, 1);
}

// 发送 HID 坐标触控屏幕
void touch(uint8_t state, int16_t hid_x, int16_t hid_y) {
  send_touch_report(state, hid_x, hid_y);
  vTaskDelay(pdMS_TO_TICKS(50));
  send_touch_report(0, 0, 0);
}


void long_touch(uint8_t state, int16_t hid_x, int16_t hid_y, uint32_t delay_ms) {
  send_touch_report(state, hid_x, hid_y);
  vTaskDelay(pdMS_TO_TICKS(delay_ms));
  send_touch_report(0, 0, 0);
}
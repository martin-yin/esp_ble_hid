// hid/hid_event.c
#include "hid_event.h"
#include "esp_hid_common.h"
#include "esp_hidd.h"
#include "esp_log.h"
#include "ble_gap.h"
#include "hid_param.h"
#include "hid_usage.h"
#include <stdio.h>

static const char *TAG = "hid_event";

void ble_hid_demo_task(void *pvParameters) {
#define SCREEN_WIDTH 1400  
#define SCREEN_HEIGHT 3400 
  char c;
  while (1) {
    c = fgetc(stdin);
    switch (c) {
    case 'k':
      uint16_t k_hid_x = (uint16_t)((float)880 * 65535 / SCREEN_WIDTH);
      uint16_t k_hid_y = (uint16_t)((float)3000 * 65535 / SCREEN_HEIGHT);
      touch(1, k_hid_x, k_hid_y);
      vTaskDelay(pdMS_TO_TICKS(100)); 
      break;
    case 'j':
      uint16_t j_hid_x = (uint16_t)((float)1770 * 65535 / SCREEN_HEIGHT);
      uint16_t j_hid_y = (uint16_t)((float)1200 * 65535 / SCREEN_WIDTH);
      touch(1, j_hid_x, j_hid_y);
      vTaskDelay(pdMS_TO_TICKS(100)); 
      break;
    // 键盘功能测试命令
    case '1': // 发送单个字母 'a'
      press_keys("Hello");
      break;
    case '2':
      press_key_combination(KEY_MOD_LSHIFT, KEY_A);
      break;
    case '3': // 发送字符串 "Hello World!"
      press_keys("Hello World!");
      break;
    case '4': // 发送 Ctrl+V
      press_key_combination(KEY_MOD_LCTRL, KEY_V);
      break;
    case 'b': // Android返回键
      send_consumer_key_report(0x0224);
      break;
    case 'm': // Android主页键
      send_consumer_key_report(0x0223);
      break;
    case 'r':
      send_consumer_key_report(0x01A2);
      break;
    case 'h': // 帮助信息
      printf("HID BLE demo help:\n");
      printf("k - touch screen at (400, 400)\n");
      printf("1 - send 'Hello'\n");
      printf("2 - send Shift+A\n");
      printf("3 - send 'Hello World!'\n");
      printf("4 - send Ctrl+V\n");
      printf("b - Android back key\n");
      printf("m - Android home key\n");
      printf("r - Android recent apps key\n");
      printf("h - show this help\n");
      break;
    default:
      break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}


void ble_hid_task_start_up(void) {
  if (s_ble_hid_param.task_hdl) {
    return;
  }

  xTaskCreate(ble_hid_demo_task, "ble_hid_demo_task", 3 * 1024,
              NULL, configMAX_PRIORITIES - 3, &s_ble_hid_param.task_hdl);
}

void ble_hid_task_shut_down(void) {
  if (s_ble_hid_param.task_hdl) {
    vTaskDelete(s_ble_hid_param.task_hdl);
    s_ble_hid_param.task_hdl = NULL;
  }
}

void hid_event_callback(void *handler_args, esp_event_base_t base,
                       int32_t id, void *event_data) {
  esp_hidd_event_t event = (esp_hidd_event_t)id;
  esp_hidd_event_data_t *param = (esp_hidd_event_data_t *)event_data;
  static const char *TAG = "HID_DEV_BLE";

  switch (event) {
  case ESP_HIDD_START_EVENT: {
    ESP_LOGI(TAG, "START");
    hid_ble_gap_adv_start();
    break;
  }
  case ESP_HIDD_CONNECT_EVENT: {
    ESP_LOGI(TAG, "CONNECT");
    break;
  }
  case ESP_HIDD_PROTOCOL_MODE_EVENT: {
    ESP_LOGI(TAG, "PROTOCOL MODE[%u]: %s", param->protocol_mode.map_index,
             param->protocol_mode.protocol_mode ? "REPORT" : "BOOT");
    break;
  }
  case ESP_HIDD_CONTROL_EVENT: {
    ESP_LOGI(TAG, "CONTROL[%u]: %sSUSPEND", param->control.map_index,
             param->control.control ? "EXIT_" : "");
    if (param->control.control) {
      // exit suspend
      ble_hid_task_start_up();
    } else {
      // suspend
      ble_hid_task_shut_down();
    }
    break;
  }
  case ESP_HIDD_OUTPUT_EVENT: {
    ESP_LOGI(TAG,
             "OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:", param->output.map_index,
             esp_hid_usage_str(param->output.usage), param->output.report_id,
             param->output.length);
    ESP_LOG_BUFFER_HEX(TAG, param->output.data, param->output.length);
    break;
  }
  case ESP_HIDD_FEATURE_EVENT: {
    ESP_LOGI(TAG, "FEATURE[%u]: %8s ID: %2u, Len: %d, Data:",
             param->feature.map_index, esp_hid_usage_str(param->feature.usage),
             param->feature.report_id, param->feature.length);
    ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
    break;
  }
  case ESP_HIDD_DISCONNECT_EVENT: {
    ESP_LOGI(TAG, "DISCONNECT: %s",
             esp_hid_disconnect_reason_str(
                 esp_hidd_dev_transport_get(param->disconnect.dev),
                 param->disconnect.reason));
    hid_ble_gap_adv_start();
    break;
  }
  case ESP_HIDD_STOP_EVENT: {
    ESP_LOGI(TAG, "STOP");
    break;
  }
  default:
    ESP_LOGI(TAG, "default event: %d", event);
    break;
  }
  return;
}
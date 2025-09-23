// hid/hid_event.c
#include "hid_event.h"
#include "esp_hid_common.h"
#include "esp_hidd.h"
#include "esp_log.h"
#include "ble_gap.h"
#include "hid_param.h"

static const char *TAG = "hid_event";
void ble_hid_demo_task(void *pvParameters) {
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
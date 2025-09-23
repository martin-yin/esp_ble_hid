// app_main.c
#include "ble_gap.h"
#include "hid_event.h"
#include "hid_param.h"
#include "hid_report_map.h"
#include "esp_hidd.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "host/ble_hs.h"
#include "gatt_services.h"
#define TAG "app_main"

static esp_hid_device_config_t hid_config = {
    .vendor_id = 0x16C0,
    .product_id = 0x05DF,
    .version = 0x0100,
    .device_name = "ESP BLE HID",
    .manufacturer_name = "Espressif",
    .serial_number = "1234567890",
    .report_maps = hid_raw_report_map,
    .report_maps_len = 1
};

void ble_hid_device_host_task(void *param) {
  ESP_LOGI(TAG, "BLE Host Task Started");
  nimble_port_run();
  nimble_port_freertos_deinit();
}

void ble_store_config_init(void);

static void ble_on_sync(void) {
    ESP_LOGI(TAG, "BLE stack synced");
    gatt_svc_init();  // 注册新 GATT 服务
    hid_ble_gap_adv_start();   // 您的现有广告启动（从 ble_gap.c）
}

void app_main(void) {
  esp_err_t ret;
  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ret = hid_ble_gap_init();
  ESP_ERROR_CHECK(ret);

  ret = hid_ble_gap_adv_init(ESP_HID_APPEARANCE_GENERIC,
                                 hid_config.device_name);
  ESP_ERROR_CHECK(ret);
  ESP_ERROR_CHECK(esp_hidd_dev_init(&hid_config, ESP_HID_TRANSPORT_BLE,
                                    hid_event_callback,
                                    &s_ble_hid_param.hid_dev));
  ble_store_config_init();
  ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
  ble_hs_cfg.sync_cb = ble_on_sync;
  ret = esp_nimble_enable(ble_hid_device_host_task);
  if (ret) {
    ESP_LOGE(TAG, "esp_nimble_enable failed: %d", ret);
  }
}
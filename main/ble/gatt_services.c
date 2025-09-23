#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "esp_log.h"
#include "hid_usage.h"  
#include "hid_param.h"  

static const char *TAG = "BLE_GATT_SVCS";

static const ble_uuid128_t hid_control_svc_uuid = 
    BLE_UUID128_INIT(0x00, 0x00, 0x15, 0x25, 0x12, 0xef, 0xde, 0x15, 0x23, 0x78, 0x5f, 0xea, 0xbc, 0xd1, 0x23, 0x00);

// 自定义写特性 UUID (128-bit)
static const ble_uuid128_t hid_command_chr_uuid = 
    BLE_UUID128_INIT(0x00, 0x00, 0x15, 0x25, 0x12, 0xef, 0xde, 0x15, 0x23, 0x78, 0x5f, 0xea, 0xbc, 0xd1, 0x23, 0x01);

// static uint16_t hid_command_chr_val_handle;

// 特性访问回调：处理写操作，解析命令并调用 HID 函数
static int hid_command_chr_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        struct os_mbuf *om = ctxt->om;
        uint16_t len = OS_MBUF_PKTLEN(om); // 获取mbuf链的总长度
        uint8_t *data = (uint8_t *)os_mbuf_pullup(om, len);
        if (len > 0 && len < 128) {  // 假设命令字符串不超过 128 字节
            char command[128];
            memcpy(command, data, len);
            command[len] = '\0';  // 终止字符串

            ESP_LOGI(TAG, "Received HID command: %s", command);

            // 解析并调用 HID 函数
            if (strncmp(command, "press:", 6) == 0) {
                press_keys(command + 6);  // 调用 press_keys("Hello")
            } else if (strncmp(command, "touch:", 6) == 0) {
                int state, x, y;
                if (sscanf(command + 6, "%d,%d,%d", &state, &x, &y) == 3) {
                    touch((uint8_t)state, (int16_t)x, (int16_t)y);  // 调用 touch(1, 300, 400)
                }
            } else if (strncmp(command, "consumer:", 9) == 0) {
                uint16_t code;
                if (sscanf(command + 9, "0x%hx", &code) == 1) {
                    send_consumer_key_report(code);  
                }
            } else {
                ESP_LOGW(TAG, "Unknown command: %s", command);
            }
            return 0;
        }
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }
    return BLE_ATT_ERR_UNLIKELY;  // 不支持其他操作
}

// GATT 服务定义
static const struct ble_gatt_svc_def gatt_services[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &hid_control_svc_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &hid_command_chr_uuid.u,
                .access_cb = hid_command_chr_access,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            {0},  // 特性结束
        }
    },
    {0},  // 服务结束
};

esp_err_t gatt_svc_init(void) {
    ble_svc_gatt_init();  // 初始化 GATT 服务
    int rc = ble_gatts_count_cfg(gatt_services);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_count_cfg failed: %d", rc);
        return ESP_FAIL;
    }
    
    rc = ble_gatts_add_svcs(gatt_services);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_add_svcs failed: %d", rc);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Custom GATT service initialized successfully");
    return ESP_OK;
}
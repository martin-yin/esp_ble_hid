#include "ble_gap.h"
#include "esp_err.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "esp_log.h"
#include "hid_usage.h"  
#include "hid_param.h"  

static const char *TAG = "BLE_GATT_SVCS";

static const ble_uuid128_t hid_control_svc_uuid = 
    BLE_UUID128_INIT(0x46, 0xe0, 0x7d, 0x77, 0xd2, 0xb4, 0xb7, 0x87, 0x81, 0x45, 0x3b, 0x64, 0x4a, 0x2a, 0xed, 0x24);

static const ble_uuid128_t hid_command_chr_uuid = 
    BLE_UUID128_INIT(0x65, 0x8d, 0x40, 0x15, 0xf4, 0xf6, 0x62, 0xb2, 0xb2, 0x45, 0x89, 0xf2, 0x34, 0xe5, 0x4a, 0xc3);


static int hid_command_chr_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        ESP_LOGI(TAG, "安卓传递过来的数据: %s", ctxt->om->om_data);
        uint8_t *data = ctxt->om->om_data;
        uint16_t data_len = ctxt->om->om_len;
        
        // 临时缓冲区，用于安全处理字符串（避免溢出）
        char cmd_buf[64] = {0};
        if (data_len >= sizeof(cmd_buf)) {
            ESP_LOGE(TAG, "Command too long (max 63 bytes)");
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        memcpy(cmd_buf, data, data_len);
        cmd_buf[data_len] = '\0';  // 手动添加字符串结束符

        ESP_LOGI(TAG, "解析之后的: %s", cmd_buf);

        if (strncmp(cmd_buf, "press:", 6) == 0) {
            const char* text = cmd_buf + 6; 
            if (text[0] == '\0') {  
                ESP_LOGW(TAG, "Invalid press command: empty text (format: press:your_text)");
            } else if (strlen(text) > 64) {  
                ESP_LOGW(TAG, "Press text too long (max 64 chars), got %d", strlen(text));
            } else {
                ESP_LOGI(TAG, "Executing press - text: %s", text);
                press_keys(text);
            }
        } 

        else if (strncmp(cmd_buf, "touch:", 6) == 0) {
            int state, x, y;
            int parse_count = sscanf(cmd_buf + 6, "%d,%d,%d", &state, &x, &y);
            if (parse_count != 3) {  
                ESP_LOGW(TAG, "Invalid touch format (expected: touch:state,x,y)");
            } else if (state < 0 || state > 1) {  
                ESP_LOGW(TAG, "Invalid touch state (must be 0 or 1), got %d", state);
            } else {
                ESP_LOGI(TAG, "Executing touch - state:%d, x:%d, y:%d", state, x, y);
                touch((uint8_t)state, (int16_t)x, (int16_t)y);  
            }
        } 

        else if (strncmp(cmd_buf, "consumer:", 9) == 0) {
            uint16_t code;
            int parse_count = sscanf(cmd_buf + 9, "0x%hx", &code);  
            if (parse_count != 1) {
                parse_count = sscanf(cmd_buf + 9, "%hu", &code); 
            }
            if (parse_count != 1) {  
                ESP_LOGW(TAG, "Invalid consumer format (expected: consumer:0xHH or consumer:NN)");
                ESP_LOGW(TAG, "Example: consumer:0xE2 (volume up) or consumer:226 (same as 0xE2)");
            } else {
                ESP_LOGI(TAG, "Executing consumer key - code:0x%04X", code);
                send_consumer_key_report(code);  
            }
        } 

        else if (strncmp(cmd_buf, "longtouch:", 10) == 0) {
            int state, x, y, delay_ms;
            int parse_count = sscanf(cmd_buf + 10, "%d,%d,%d,%d", &state, &x, &y, &delay_ms);
            if (parse_count != 4) { 
                ESP_LOGW(TAG, "Invalid longtouch format (expected: longtouch:state,x,y,delay_ms)");
            } else if (state < 0 || state > 1) { 
                ESP_LOGW(TAG, "Invalid longtouch state (must be 0 or 1), got %d", state);
            } else if (delay_ms <= 0) {  
                ESP_LOGW(TAG, "Invalid delay_ms (must be >0), got %d", delay_ms);
            } else {
                ESP_LOGI(TAG, "Executing longtouch - state:%d, x:%d, y:%d, delay:%dms", 
                        state, x, y, delay_ms);
                long_touch((uint8_t)state, (int16_t)x, (int16_t)y, (uint32_t)delay_ms);  // 调用长按函数
            }
        } 

        else if (strncmp(cmd_buf, "combination:", 12) == 0) {
            uint8_t modifier;
            uint8_t key;
            int parse_count = sscanf(cmd_buf + 12, "%hhu,%hhu", &modifier, &key);
            if (parse_count != 2) {  
                ESP_LOGW(TAG, "Invalid combination format (expected: combination:modifier,key)");
            } else {
                ESP_LOGI(TAG, "Executing key combination - modifier:0x%02X, key:0x%02X", 
                        modifier, key);
                press_key_combination(modifier, key); 
            }
        }

        else if (strncmp(cmd_buf, "disconnect:", 11) == 0) {
            if (current_conn_handle != 0) {
                // 调用断开函数，原因使用 "用户终止连接"
                int rc = ble_gap_terminate(current_conn_handle, BLE_GAP_EVENT_DISCONNECT);
                if (rc != 0) {
                    ESP_LOGE(TAG, "disconnect failed: %d", rc);
                } else {
                    ESP_LOGI(TAG, "disconnect success: %d", current_conn_handle);
                }
            } else {
                ESP_LOGW(TAG, "no active connection");
            }
        }

        else {
            ESP_LOGW(TAG, "Unknown command: %s (supported: press,touch,consumer,longtouch,combination)", cmd_buf);
        }

 
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }
    return BLE_ATT_ERR_UNLIKELY; 
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
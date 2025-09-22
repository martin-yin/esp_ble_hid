set(srcs 
    "app_main.c"
    "ble/ble_gap.c"
    "hid/hid_report_map.c"
    "ble/hid_param.c"       # 修正路径：从 hid/hid_param.c 改为 ble/hid_param.c
    "hid/hid_event.c"       # 新增
)

idf_component_register(SRCS ${srcs}
                    PRIV_REQUIRES bt esp_hid nvs_flash  
                    INCLUDE_DIRS "." "ble" "hid")
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define PROFILE_NUM                 1
#define PROFILE_APP_IDX             0
#define INVALID_HANDLE              0

#define REMOTE_SERVICE_UUID         0x180F
#define REMOTE_NOTIFY_CHAR_UUID     0x2A19

struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t char_handle;
    esp_bd_addr_t remote_bda;
};

class BleClient {
public:
    static gattc_profile_inst profileInstances[PROFILE_NUM];
    static esp_ble_scan_params_t bleScanParams;

    static void initializeBluetoothClient();

private:
    static const char *BT_CLIENT_TAG;
    static const char *REMOTE_DEVICE_NAME;
    static esp_gattc_char_elem_t *charElemResult;
    static esp_gattc_descr_elem_t *descrElemResult;
    static bool _isConnected;
    static bool _getServer;

    static void _gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *gapParam);
    static void _gattcEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_interface_type, esp_ble_gattc_cb_param_t *gattcParam);
    static void _gattcProfileEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

    static bool _logIfThrowsError(esp_err_t error_code, const char *message, bool error_string = false);
};


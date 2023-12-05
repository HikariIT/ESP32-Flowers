#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#define BT_SERVER_TAG "BluetoothServer"

#define PROFILE_NUM                 1
#define PROFILE_APP_IDX             0
#define ESP_APP_ID                  0x55
#define SAMPLE_DEVICE_NAME          "ESP_GATTS_DEMO"
#define SVC_INST_ID                 0

/* The max length of characteristic value. When the GATT client performs a write or prepare write operation,
*  the data length must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
*/
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)

void ble_server_init();

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *gap_param);
static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_interface_type, esp_ble_gatts_cb_param_t *gatts_param);

enum DbIndex {
    SERVICE_DECL,

    CHARACTERISTIC_A_DECL,
    CHARACTERISTIC_A_VAL,
    CHARACTERISTIC_A_CCCD,

    CHARACTERISTIC_B_DECL,
    CHARACTERISTIC_B_VAL,

    CHARACTERISTIC_C_DECL,
    CHARACTERISTIC_C_VAL,

    DB_ATTRIBUTE_COUNT
};

inline const char* dbIndexToString(int i);

struct prepare_type_env_t {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

/*
class BleServer {
public:
    BleServer();

private:
    bool advertising_configuring = false;
    bool scan_response_configuring = false;

    static const uint16_t ESP_APP_ID        = 0x55;
    static const char* SAMPLE_DEVICE_NAME   = "ESP_GATTS_DEMO";
    static const char* BT_SERVER_TAG        = "BluetoothServer";

    static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *gap_param);
    static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_interface_type, esp_ble_gatts_cb_param_t *gatts_param);
};*/
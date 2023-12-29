#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>

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

#define PROFILE_NUM                 1
#define PROFILE_APP_IDX             0
#define SVC_INST_ID                 0

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE        1024
#define CHAR_DECLARATION_SIZE       (sizeof(uint8_t))

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

inline const char* dbIndexToString(int i);

class BleServer {
public:
    static void initializeBluetoothServer();

private:
    static const uint16_t ESP_APP_ID;
    static const char* BT_SERVER_TAG;
    static const char* SAMPLE_DEVICE_NAME;

    static bool advertisingConfiguring;
    static bool scanResponseConfiguring;
    static uint16_t basicProfileTable[DbIndex::DB_ATTRIBUTE_COUNT];
    static gatts_profile_inst profileInstances[PROFILE_NUM];
    static std::unordered_map<int, int> handleToDbIndexMap;
    static prepare_type_env_t* prepareWriteEnv;
    static esp_ble_adv_params_t advertisingParams;
    static esp_ble_adv_data_t scanResponseData;
    static esp_ble_adv_data_t advertisingData;
    static uint8_t serviceUUID[16];

    static void _gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *gap_param);
    static void _gattsProfileEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    static void _gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_interface_type, esp_ble_gatts_cb_param_t *gatts_param);

    static void _handleGattsWriteEvent(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    static void _handleGattsPrepareWriteEvent(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    static void _handleGattsExecWriteEvent(esp_ble_gatts_cb_param_t *param);
    static void _handleGattsConnectEvent(esp_ble_gatts_cb_param_t *param);
    static void _handleGattsCreateAttributeTableEvent(esp_ble_gatts_cb_param_t *param);

    static bool _logIfThrowsError(esp_err_t error_code, const char *message, bool error_string = false);
    static const char* _getCharacteristicName(uint16_t handle);
};
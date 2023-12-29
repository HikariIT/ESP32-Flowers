#include "ble_server.hpp"

// region BLE Server - Static variables initialization

const char *BleServer::BT_SERVER_TAG = "BluetoothServer";
const char *BleServer::SAMPLE_DEVICE_NAME = "ESP32 BLE Server";
const uint16_t BleServer::ESP_APP_ID = 0x55;

bool BleServer::advertisingConfiguring = false;
bool BleServer::scanResponseConfiguring = false;

gatts_profile_inst BleServer::profileInstances[] = {
        [PROFILE_APP_IDX] = {
                .gatts_cb = _gattsProfileEventHandler,
                .gatts_if = ESP_GATT_IF_NONE,
        },
};

uint8_t BleServer::serviceUUID[16] = {
        0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

esp_ble_adv_data_t BleServer::advertisingData = {
        .set_scan_rsp        = false,
        .include_name        = true,
        .include_txpower     = true,
        .min_interval        = 0x0006,
        .max_interval        = 0x0010,
        .appearance          = 0x00,
        .manufacturer_len    = 0,
        .p_manufacturer_data = nullptr,
        .service_data_len    = 0,
        .p_service_data      = nullptr,
        .service_uuid_len    = sizeof(serviceUUID),
        .p_service_uuid      = serviceUUID,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

esp_ble_adv_data_t BleServer::scanResponseData = {
        .set_scan_rsp        = true,
        .include_name        = true,
        .include_txpower     = true,
        .min_interval        = 0x0006,
        .max_interval        = 0x0010,
        .appearance          = 0x00,
        .manufacturer_len    = 0,
        .p_manufacturer_data = nullptr,
        .service_data_len    = 0,
        .p_service_data      = nullptr,
        .service_uuid_len    = sizeof(serviceUUID),
        .p_service_uuid      = serviceUUID,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

esp_ble_adv_params_t BleServer::advertisingParams = {
        .adv_int_min         = 0x20,
        .adv_int_max         = 0x40,
        .adv_type            = ADV_TYPE_IND,
        .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
        .channel_map         = ADV_CHNL_ALL,
        .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

uint16_t BleServer::basicProfileTable[DbIndex::DB_ATTRIBUTE_COUNT] = {0};
std::unordered_map<int, int> BleServer::handleToDbIndexMap = {};
prepare_type_env_t* BleServer::prepareWriteEnv = new prepare_type_env_t {
        .prepare_buf = nullptr,
        .prepare_len = 0,
};

// endregion

// region Utility functions

const char *dbIndexToString(int i) {
    switch (i)
    {
        case 0:     return "Service Declaration";
        case 1:     return "Characteristic A Declaration";
        case 2:     return "Characteristic A Value";
        case 3:     return "Characteristic A CCCD";
        case 4:     return "Characteristic B Declaration";
        case 5:     return "Characteristic B Value";
        case 6:     return "Characteristic C Declaration";
        case 7:     return "Characteristic C Value";
        default:    return "[Unknown DbIndex]";
    }
}

// endregion

// region Service and Characteristic Definitions

static const uint16_t SERVICE_UUID                 = 0x00FF;
static const uint16_t CHARACTERISTIC_A_UUID        = 0xFF01;
static const uint16_t CHARACTERISTIC_B_UUID        = 0xFF02;
static const uint16_t CHARACTERISTIC_C_UUID        = 0xFF03;

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read                = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t heart_measurement_ccc[2]      = { 0x00, 0x00 };
static const uint8_t char_value[4]                 = { 0x11, 0x22, 0x33, 0x44 };

/* Structure of the Database is as follows:
 * ------------------------------------------------------------
 *     Service Declaration
 *     ├── Characteristic Declaration (A)
 *     │   ├── Characteristic Value (A) - READ / WRITE / NOTIFY
 *     │   └── Client Characteristic Descriptor Declaration (A)
 *     ├── Characteristic Declaration (B)
 *     │   └── Characteristic Value (B) - WRITE
 *     └── Characteristic Declaration (C)
 *         └── Characteristic Value (C) - READ
 * ------------------------------------------------------------
 */

static const esp_gatts_attr_db_t gatt_db[DbIndex::DB_ATTRIBUTE_COUNT] = {
    [DbIndex::SERVICE_DECL] = {
        .attr_control = {
            .auto_rsp       = ESP_GATT_AUTO_RSP
        },
        .att_desc = {
            .uuid_length    = ESP_UUID_LEN_16,
            .uuid_p         = (uint8_t *) &primary_service_uuid,
            .perm           = ESP_GATT_PERM_READ,
            .max_length     = sizeof(uint16_t),
            .length         = sizeof(uint16_t),
            .value          = (uint8_t *) &SERVICE_UUID,
        }
    },
    [DbIndex::CHARACTERISTIC_A_DECL] = {
        .attr_control = {
            .auto_rsp       = ESP_GATT_AUTO_RSP
        },
        .att_desc = {
            .uuid_length    = ESP_UUID_LEN_16,
            .uuid_p         = (uint8_t*) &character_declaration_uuid,
            .perm           = ESP_GATT_PERM_READ,
            .max_length     = CHAR_DECLARATION_SIZE,
            .length         = CHAR_DECLARATION_SIZE,
            .value          = (uint8_t*) &char_prop_read_write_notify,
        }
    },
    [DbIndex::CHARACTERISTIC_A_VAL] = {
        .attr_control = {
            .auto_rsp       = ESP_GATT_AUTO_RSP
        },
        .att_desc = {
            .uuid_length    = ESP_UUID_LEN_16,
            .uuid_p         = (uint8_t*) &CHARACTERISTIC_A_UUID,
            .perm           = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            .max_length     = GATTS_DEMO_CHAR_VAL_LEN_MAX,
            .length         = sizeof(char_value),
            .value          = (uint8_t*) char_value,
        }
    },
    [DbIndex::CHARACTERISTIC_A_CCCD] = {
        .attr_control = {
            .auto_rsp       = ESP_GATT_AUTO_RSP
        },
        .att_desc = {
            .uuid_length    = ESP_UUID_LEN_16,
            .uuid_p         = (uint8_t*) &character_client_config_uuid,
            .perm           = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            .max_length     = sizeof(uint16_t),
            .length         = sizeof(uint16_t),
            .value          = (uint8_t*) heart_measurement_ccc,
        }
    },
    [DbIndex::CHARACTERISTIC_B_DECL] = {
        .attr_control = {
            .auto_rsp       = ESP_GATT_AUTO_RSP
        },
        .att_desc = {
            .uuid_length    = ESP_UUID_LEN_16,
            .uuid_p         = (uint8_t*) &character_declaration_uuid,
            .perm           = ESP_GATT_PERM_READ,
            .max_length     = CHAR_DECLARATION_SIZE,
            .length         = CHAR_DECLARATION_SIZE,
            .value          = (uint8_t*) &char_prop_write,
        }
    },

    [DbIndex::CHARACTERISTIC_B_VAL] = {
        .attr_control = {
            .auto_rsp       = ESP_GATT_AUTO_RSP
        },
        .att_desc = {
            .uuid_length    = ESP_UUID_LEN_16,
            .uuid_p         = (uint8_t*) &CHARACTERISTIC_B_UUID,
            .perm           = ESP_GATT_PERM_WRITE,
            .max_length     = GATTS_DEMO_CHAR_VAL_LEN_MAX,
            .length         = sizeof(char_value),
            .value          = (uint8_t*) char_value,
        }
    },

    [DbIndex::CHARACTERISTIC_C_DECL] = {
        .attr_control = {
            .auto_rsp       = ESP_GATT_AUTO_RSP
        },
        .att_desc = {
            .uuid_length    = ESP_UUID_LEN_16,
            .uuid_p         = (uint8_t*) &character_declaration_uuid,
            .perm           = ESP_GATT_PERM_READ,
            .max_length     = CHAR_DECLARATION_SIZE,
            .length         = CHAR_DECLARATION_SIZE,
            .value          = (uint8_t*) &char_prop_read,
        }
    },

    [DbIndex::CHARACTERISTIC_C_VAL] = {
        .attr_control = {
            .auto_rsp       = ESP_GATT_AUTO_RSP
        },
        .att_desc = {
            .uuid_length    = ESP_UUID_LEN_16,
            .uuid_p         = (uint8_t*) &CHARACTERISTIC_C_UUID,
            .perm           = ESP_GATT_PERM_READ,
            .max_length     = GATTS_DEMO_CHAR_VAL_LEN_MAX,
            .length         = sizeof(char_value),
            .value          = (uint8_t*) char_value,
        }
    },
};

// endregion

// region BLE Server - Public methods

void BleServer::initializeBluetoothServer() {
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_err_t error_code;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    // Initialize bluetooth controller with default config
    error_code = esp_bt_controller_init(&bt_cfg);
    if (error_code) {
        ESP_LOGE(BleServer::BT_SERVER_TAG, "Bluetooth controller initialization failed. Error: %s", esp_err_to_name(error_code));
        return;
    }

    // Set bluetooth controller mode to BLE only and enable it
    error_code = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (error_code) {
        ESP_LOGE(BleServer::BT_SERVER_TAG, "Bluetooth controller initialization failed. Error: %s", esp_err_to_name(error_code));
        return;
    }

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();

    // Initialize bluedroid stack with default config
    error_code = esp_bluedroid_init_with_cfg(&bluedroid_cfg);
    if (error_code) {
        ESP_LOGE(BleServer::BT_SERVER_TAG, "Bluedroid initialization failed: %s", esp_err_to_name(error_code));
        return;
    }

    // Enable bluedroid stack
    error_code = esp_bluedroid_enable();
    if (error_code) {
        ESP_LOGE(BleServer::BT_SERVER_TAG, "Bluedroid initialization failed: %s", esp_err_to_name(error_code));
        return;
    }

    // Here we are starting to use our own callbacks ----------

    // Register callback function to the GATTS module
    error_code = esp_ble_gatts_register_callback(BleServer::_gattsEventHandler);
    if (error_code) {
        ESP_LOGE(BleServer::BT_SERVER_TAG, "GATTS callback register failed. Error: %x", error_code);
        return;
    }

    // Register callback function to the GAP module
    error_code = esp_ble_gap_register_callback(BleServer::_gapEventHandler);
    if (error_code) {
        ESP_LOGE(BleServer::BT_SERVER_TAG, "GAP callback register failed. Error: %x", error_code);
        return;
    }

    // Register app
    error_code = esp_ble_gatts_app_register(ESP_APP_ID);
    if (error_code) {
        ESP_LOGE(BleServer::BT_SERVER_TAG, "GATTS app register failed. Error: %x", error_code);
        return;
    }

    // Set Maximum Transmission Unit (MTU) size
    error_code = esp_ble_gatt_set_local_mtu(500);
    if (error_code) {
        ESP_LOGE(BleServer::BT_SERVER_TAG, "Setting of local MTU failed. Error: %x", error_code);
    }
}

// endregion

// region BLE Server - Event handlers

void BleServer::_gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *gap_param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            advertisingConfiguring = false;
            if (!scanResponseConfiguring) {
                ESP_LOGI(BleServer::BT_SERVER_TAG, "Advertising data set complete, starting advertising...");
                esp_ble_gap_start_advertising(&advertisingParams);
            }
            break;

        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            scanResponseConfiguring = false;
            if (!advertisingConfiguring) {
                ESP_LOGI(BleServer::BT_SERVER_TAG, "Advertising data set complete, starting advertising...");
                esp_ble_gap_start_advertising(&advertisingParams);
            }
            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (gap_param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(BleServer::BT_SERVER_TAG, "Advertising start failed");
            } else {
                ESP_LOGI(BleServer::BT_SERVER_TAG, "Advertising start successful");
            }
            break;

        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (gap_param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(BleServer::BT_SERVER_TAG, "Advertising stop failed");
            } else {
                ESP_LOGI(BleServer::BT_SERVER_TAG, "Advertising stop successful");
            }
            break;

        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(BleServer::BT_SERVER_TAG, "Updating connection params");
            ESP_LOGI(BleServer::BT_SERVER_TAG, "--------------------------------------");
            ESP_LOGI(BleServer::BT_SERVER_TAG, "Status: %d", gap_param->update_conn_params.status);
            ESP_LOGI(BleServer::BT_SERVER_TAG, "Min interval: %d", gap_param->update_conn_params.min_int);
            ESP_LOGI(BleServer::BT_SERVER_TAG, "Max interval: %d", gap_param->update_conn_params.max_int);
            ESP_LOGI(BleServer::BT_SERVER_TAG, "Current interval: %d", gap_param->update_conn_params.conn_int);
            ESP_LOGI(BleServer::BT_SERVER_TAG, "Latency: %d", gap_param->update_conn_params.latency);
            ESP_LOGI(BleServer::BT_SERVER_TAG, "Timeout: %d", gap_param->update_conn_params.timeout);
            ESP_LOGI(BleServer::BT_SERVER_TAG, "--------------------------------------");
            break;

        default:
            break;
    }
}

void BleServer::_gattsProfileEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
        case ESP_GATTS_REG_EVT:
            if (_logIfThrowsError(esp_ble_gap_set_device_name(SAMPLE_DEVICE_NAME), "Set device name operation failed"))
                return;

            if (_logIfThrowsError(esp_ble_gap_config_adv_data(&advertisingData),
                                  "Configuration of advertising data failed"))
                return;
            advertisingConfiguring = true;

            if (_logIfThrowsError(esp_ble_gap_config_adv_data(&scanResponseData),
                                  "Configuration of scan response data failed"))
                return;
            scanResponseConfiguring = true;

            if (_logIfThrowsError(
                    esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, DbIndex::DB_ATTRIBUTE_COUNT, SVC_INST_ID),
                    "Creation of attribute table failed"))
                return;
            break;

        case ESP_GATTS_READ_EVT:
            ESP_LOGI(BleServer::BT_SERVER_TAG, "GATTS Read event received for %s",
                     BleServer::_getCharacteristicName(param->read.handle));
            break;

        case ESP_GATTS_WRITE_EVT:
            _handleGattsWriteEvent(gatts_if, param);
            break;

        case ESP_GATTS_EXEC_WRITE_EVT:
            _handleGattsExecWriteEvent(param);
            break;

        case ESP_GATTS_MTU_EVT:
            ESP_LOGI(BleServer::BT_SERVER_TAG, "MTU Event received. MTU: %d", param->mtu.mtu);
            break;

        case ESP_GATTS_CONF_EVT:
            ESP_LOGI(BleServer::BT_SERVER_TAG, "GATTS Confirm Event received for %s. Status: %d",
                     BleServer::_getCharacteristicName(param->conf.handle), param->conf.status);
            break;

        case ESP_GATTS_START_EVT:
            ESP_LOGI(BleServer::BT_SERVER_TAG, "GATTS Start Event received for %s. Status: %d",
                     BleServer::_getCharacteristicName(param->start.service_handle), param->start.status);
            break;

        case ESP_GATTS_CONNECT_EVT:
            _handleGattsConnectEvent(param);
            break;

        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(BleServer::BT_SERVER_TAG, "GATTS Disconnect Event received. Reason: 0x%x", param->disconnect.reason);
            esp_ble_gap_start_advertising(&advertisingParams);
            break;

        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
            _handleGattsCreateAttributeTableEvent(param);
            break;

        case ESP_GATTS_STOP_EVT:
        case ESP_GATTS_OPEN_EVT:
        case ESP_GATTS_CANCEL_OPEN_EVT:
        case ESP_GATTS_CLOSE_EVT:
        case ESP_GATTS_LISTEN_EVT:
        case ESP_GATTS_CONGEST_EVT:
        case ESP_GATTS_UNREG_EVT:
        case ESP_GATTS_DELETE_EVT:
        default:
            break;
    }
}

void BleServer::_gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_interface_type, esp_ble_gatts_cb_param_t *gatts_param) {
    // If event is register event, store the gatts_if for each profile
    if (event == ESP_GATTS_REG_EVT) {
        if (gatts_param->reg.status == ESP_GATT_OK) {
            profileInstances[PROFILE_APP_IDX].gatts_if = gatts_interface_type;
            ESP_LOGI(BleServer::BT_SERVER_TAG, "App registration successful. App ID: %04x, Interface type: %d", gatts_param->reg.app_id, gatts_interface_type);
        }
        else {
            ESP_LOGE(BleServer::BT_SERVER_TAG, "App registration failed. App ID: %04x, Status: %d", gatts_param->reg.app_id, gatts_param->reg.status);
            return;
        }
    }

    for (auto & profile : profileInstances) {
        // If interface is ESP_GATT_IF_NONE, we call callback functions for each profile in heart_rate_profile_tab
        if (gatts_interface_type == ESP_GATT_IF_NONE || gatts_interface_type == profile.gatts_if) {
            if (profile.gatts_cb) {
                profile.gatts_cb(event, gatts_interface_type, gatts_param);
            }
        }
    }
}

// endregion

// region BLE Server - GATTS Event Handlers

void BleServer::_handleGattsWriteEvent(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    if (!param->write.is_prep){
        ESP_LOGI(BleServer::BT_SERVER_TAG, "GATTS Write event received for %s. Length: %d",
                 BleServer::_getCharacteristicName(param->write.handle), param->write.len);
        ESP_LOGI(BleServer::BT_SERVER_TAG, "Received following bytes: ");
        esp_log_buffer_hex(BleServer::BT_SERVER_TAG, param->write.value, param->write.len);

        if (BleServer::handleToDbIndexMap[param->write.handle] == DbIndex::CHARACTERISTIC_A_CCCD && param->write.len == 2){
            uint16_t descriptorValue = param->write.value[1] << 8 | param->write.value[0];
            switch (descriptorValue) {
                case 0x0000:
                    ESP_LOGI(BleServer::BT_SERVER_TAG, "Notify / indicate disabled");
                    break;
                case 0x0001:
                    ESP_LOGI(BleServer::BT_SERVER_TAG, "Notify enabled");
                    uint8_t notify_data[15];
                    for (int i = 0; i < sizeof(notify_data); i++)
                        notify_data[i] = i % 0xff;
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, basicProfileTable[DbIndex::CHARACTERISTIC_A_VAL],
                                                sizeof(notify_data), notify_data, false);
                    break;
                case 0x0002:
                    ESP_LOGI(BleServer::BT_SERVER_TAG, "Indicate enabled");
                    uint8_t indicate_data[15];
                    for (int i = 0; i < sizeof(indicate_data); i++)
                        indicate_data[i] = i % 0xff;
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, basicProfileTable[DbIndex::CHARACTERISTIC_A_VAL],
                                                sizeof(indicate_data), indicate_data, true);
                    break;
                default:
                    ESP_LOGI(BleServer::BT_SERVER_TAG, "Unknown descriptor value received");
                    break;
            }
        }

        ESP_LOGI(BleServer::BT_SERVER_TAG, "--------------------------------------");
        if (param->write.need_rsp) {
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, nullptr);
        }
    } else {
        _handleGattsPrepareWriteEvent(gatts_if, param);
    }
}

void BleServer::_handleGattsPrepareWriteEvent(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(BleServer::BT_SERVER_TAG, "GATTS Prepare Write event received for %s. Length: %d",
             BleServer::_getCharacteristicName(param->write.handle), param->write.len);

    esp_gatt_status_t status = ESP_GATT_OK;
    if (prepareWriteEnv->prepare_buf == nullptr) {
        prepareWriteEnv->prepare_buf = (uint8_t*) malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
        prepareWriteEnv->prepare_len = 0;

        if (prepareWriteEnv->prepare_buf == nullptr) {
            ESP_LOGE(BleServer::BT_SERVER_TAG, "GATTS Prepare Write: Failed to allocate buffer memory");
            status = ESP_GATT_NO_RESOURCES;
        }
    } else {
        if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_OFFSET;
        } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_ATTR_LEN;
        }
    }

    if (param->write.need_rsp) {
        auto gatt_rsp = (esp_gatt_rsp_t*) malloc(sizeof(esp_gatt_rsp_t));
        if (gatt_rsp != nullptr) {
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);

            if (response_err != ESP_OK)
                ESP_LOGE(BleServer::BT_SERVER_TAG, "GATTS Prepare Write: Error while sending response");
            free(gatt_rsp);
        } else {
            ESP_LOGE(BleServer::BT_SERVER_TAG, "GATTS Prepare Write: Failed to allocate memory for response");
            status = ESP_GATT_NO_RESOURCES;
        }
    }

    if (status != ESP_GATT_OK){
        return;
    }

    memcpy(prepareWriteEnv->prepare_buf + param->write.offset,
           param->write.value,
           param->write.len);
    prepareWriteEnv->prepare_len += param->write.len;

}

void BleServer::_handleGattsExecWriteEvent(esp_ble_gatts_cb_param_t *param) {
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC && prepareWriteEnv->prepare_buf){
        ESP_LOGI(BleServer::BT_SERVER_TAG, "GATTS Prepare Write executed. Length: %d.", prepareWriteEnv->prepare_len);
        ESP_LOGI(BleServer::BT_SERVER_TAG, "Received following bytes:");
                esp_log_buffer_hex(BleServer::BT_SERVER_TAG, prepareWriteEnv->prepare_buf, prepareWriteEnv->prepare_len);
        ESP_LOGI(BleServer::BT_SERVER_TAG, "--------------------------------------");
    } else {
        ESP_LOGI(BleServer::BT_SERVER_TAG, "GATTS Prepare Write cancelled");
    }
    if (prepareWriteEnv->prepare_buf) {
        free(prepareWriteEnv->prepare_buf);
        prepareWriteEnv->prepare_buf = nullptr;
    }
    prepareWriteEnv->prepare_len = 0;
}

void BleServer::_handleGattsConnectEvent(esp_ble_gatts_cb_param_t *param) {
    ESP_LOGI(BleServer::BT_SERVER_TAG, "GATTS Connect Event received. Connection ID: %d, Remote Bluetooth device address:", param->connect.conn_id);
            esp_log_buffer_hex(BleServer::BT_SERVER_TAG, param->connect.remote_bda, 6);

    esp_ble_conn_update_params_t conn_params = {};
    memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));

    conn_params.latency = 0;
    conn_params.max_int = 0x20;
    conn_params.min_int = 0x10;
    conn_params.timeout = 400;

    esp_ble_gap_update_conn_params(&conn_params);
}

void BleServer::_handleGattsCreateAttributeTableEvent(esp_ble_gatts_cb_param_t *param) {
    if (param->add_attr_tab.status != ESP_GATT_OK){
        ESP_LOGE(BleServer::BT_SERVER_TAG, "Attribute table creation failed. Error code: 0x%x",
                 param->add_attr_tab.status);
    }
    else if (param->add_attr_tab.num_handle != DbIndex::DB_ATTRIBUTE_COUNT){
        ESP_LOGE(BleServer::BT_SERVER_TAG, "Attribute table creation abnormal, number of the attribute handles (%d) doesn't match DB_ATTRIBUTE_COUNT (%d)",
                 param->add_attr_tab.num_handle, DbIndex::SERVICE_DECL);
    }
    else {
        ESP_LOGI(BleServer::BT_SERVER_TAG, "Attribute table created successfully, number of attribute handles: %d",
                 param->add_attr_tab.num_handle);
        memcpy(basicProfileTable, param->add_attr_tab.handles, sizeof(basicProfileTable));
        esp_ble_gatts_start_service(basicProfileTable[DbIndex::SERVICE_DECL]);

        ESP_LOGI(BleServer::BT_SERVER_TAG, "Handles");
        ESP_LOGI(BleServer::BT_SERVER_TAG, "------------------------------------");
        for (int i = 0; i < DbIndex::DB_ATTRIBUTE_COUNT; ++i) {
            ESP_LOGI(BleServer::BT_SERVER_TAG, "[%d] %s", basicProfileTable[i], dbIndexToString(i));
            BleServer::handleToDbIndexMap[basicProfileTable[i]] = i;
        }
        ESP_LOGI(BleServer::BT_SERVER_TAG, "------------------------------------");
    }
}

// endregion

// region BLE Server - Helpers

const char* BleServer::_getCharacteristicName(uint16_t handle) {
    auto it = BleServer::handleToDbIndexMap.find(handle);
    return it != BleServer::handleToDbIndexMap.end() ? dbIndexToString(it->second) : "Unknown";
}

bool BleServer::_logIfThrowsError(esp_err_t error_code, const char* message, bool error_string)
{
    if (error_code) {
        if (error_string)
            ESP_LOGE(BleServer::BT_SERVER_TAG, "%s. Error: %s", message, esp_err_to_name(error_code));
        else
            ESP_LOGE(BleServer::BT_SERVER_TAG, "%s. Error: 0x%x", message, error_code);
        return true;
    }
    return false;
}

// endregion

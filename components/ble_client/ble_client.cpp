#include "ble_client.hpp"

// Initialize static variables
const char* BleClient::BT_CLIENT_TAG                    = "BluetoothClient";
const char* BleClient::REMOTE_DEVICE_NAME               = "iTAG            ";
esp_gattc_char_elem_t *BleClient::charElemResult      = nullptr;
esp_gattc_descr_elem_t *BleClient::descrElemResult    = nullptr;
bool BleClient::_isConnected                            = false;
bool BleClient::_getServer                              = false;

static esp_bt_uuid_t remote_filter_service_uuid = {
        .len = ESP_UUID_LEN_16,
        .uuid = {.uuid16 = REMOTE_SERVICE_UUID},
};

static esp_bt_uuid_t remote_filter_char_uuid = {
        .len = ESP_UUID_LEN_16,
        .uuid = {.uuid16 = REMOTE_NOTIFY_CHAR_UUID},
};

static esp_bt_uuid_t notify_descr_uuid = {
        .len = ESP_UUID_LEN_16,
        .uuid = {.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG},
};

esp_ble_scan_params_t BleClient::bleScanParams = {
        .scan_type              = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval          = 0x50,
        .scan_window            = 0x30,
        .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};


gattc_profile_inst BleClient::profileInstances[PROFILE_NUM] = {
        [PROFILE_APP_IDX] = {
                .gattc_cb = _gattcProfileEventHandler,
                .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
        },
};


void BleClient::initializeBluetoothClient() {
    esp_err_t error_code;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    // Initialize bluetooth controller with default config
    error_code = esp_bt_controller_init(&bt_cfg);
    if (error_code) {
        ESP_LOGE(BT_CLIENT_TAG, "Bluetooth controller initialization failed. Error: %s", esp_err_to_name(error_code));
        return;
    }

    // Set bluetooth controller mode to BLE only and enable it
    error_code = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (error_code) {
        ESP_LOGE(BT_CLIENT_TAG, "Bluetooth controller initialization failed. Error: %s", esp_err_to_name(error_code));
        return;
    }

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();

    // Initialize bluedroid stack with default config
    error_code = esp_bluedroid_init_with_cfg(&bluedroid_cfg);
    if (error_code) {
        ESP_LOGE(BT_CLIENT_TAG, "Bluedroid initialization failed: %s", esp_err_to_name(error_code));
        return;
    }

    // Enable bluedroid stack
    error_code = esp_bluedroid_enable();
    if (error_code) {
        ESP_LOGE(BT_CLIENT_TAG, "Bluedroid initialization failed: %s", esp_err_to_name(error_code));
        return;
    }

    // Register the callback function to the GAP module
    error_code = esp_ble_gap_register_callback(BleClient::_gapEventHandler);
    if (error_code){
        ESP_LOGE(BT_CLIENT_TAG,  "GAP callback register failed. Error: %x", error_code);
        return;
    }

    // Register the callback function to the GATTC module
    error_code = esp_ble_gattc_register_callback(BleClient::_gattcEventHandler);
    if(error_code){
        ESP_LOGE(BT_CLIENT_TAG, "GATTC callback register failed. Error: %x", error_code);
        return;
    }

    error_code = esp_ble_gattc_app_register(PROFILE_APP_IDX);
    if (error_code){
        ESP_LOGE(BT_CLIENT_TAG,  "GATTS app register failed. Error: %x", error_code);
    }

    // Set Maximum Transmission Unit (MTU) size
    error_code = esp_ble_gatt_set_local_mtu(500);
    if (error_code) {
        ESP_LOGE(BT_CLIENT_TAG, "Setting of local MTU failed. Error: %x", error_code);
    }

    esp_ble_gattc_read_char(profileInstances[PROFILE_APP_IDX].gattc_if, profileInstances[PROFILE_APP_IDX].conn_id, profileInstances[PROFILE_APP_IDX].char_handle, ESP_GATT_AUTH_REQ_NONE);
}

void BleClient::_gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *gapParam) {
    uint32_t SCAN_DURATION_IN_SECONDS = 30;
    esp_ble_gap_cb_param_t *scan_result;
    uint8_t adv_name_len = 0;
    uint8_t *adv_name;

    switch (event) {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
            ESP_LOGI(BT_CLIENT_TAG, "Scan parameters set, starting scanning");
            esp_ble_gap_start_scanning(SCAN_DURATION_IN_SECONDS);
            break;

        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
            if (gapParam->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(BT_CLIENT_TAG, "Scanning start failed. Error: %s", esp_err_to_name(gapParam->scan_start_cmpl.status));
            } else {
                ESP_LOGI(BT_CLIENT_TAG, "Scanning started");
            }
            break;

        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            scan_result = static_cast<esp_ble_gap_cb_param_t *>(gapParam);
            if (scan_result->scan_rst.search_evt != ESP_GAP_SEARCH_INQ_RES_EVT) {
                break;
            }

            vTaskDelay(10 / portTICK_PERIOD_MS);
            ESP_LOGI(BT_CLIENT_TAG, "Bluetooth Device address: ");
            esp_log_buffer_hex(BT_CLIENT_TAG, scan_result->scan_rst.bda, 6);

            ESP_LOGI(BT_CLIENT_TAG, "Advertising data length: %d", scan_result->scan_rst.adv_data_len);
            ESP_LOGI(BT_CLIENT_TAG, "Scan response data length: %d", scan_result->scan_rst.scan_rsp_len);
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);

            ESP_LOGI(BT_CLIENT_TAG, "Device name length: %d.", adv_name_len);
            esp_log_buffer_char(BT_CLIENT_TAG, adv_name, adv_name_len);
            ESP_LOGI(BT_CLIENT_TAG, "--------------------------------------");

            if (adv_name == nullptr) {
                break;
            }

            ESP_LOGI(BT_CLIENT_TAG, "Found a device.");
            if (strlen(REMOTE_DEVICE_NAME) == adv_name_len && strncmp((char *)adv_name, REMOTE_DEVICE_NAME, adv_name_len) == 0) {
                ESP_LOGI(BT_CLIENT_TAG, "Found target device with matching name: %s", REMOTE_DEVICE_NAME);
                if (_isConnected)
                    break;

                _isConnected = true;
                ESP_LOGI(BT_CLIENT_TAG, "Connected to the remote device.");
                esp_ble_gap_stop_scanning();
                esp_ble_gattc_open(profileInstances[PROFILE_APP_IDX].gattc_if, scan_result->scan_rst.bda, scan_result->scan_rst.ble_addr_type, true);
            }
            break;

        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
            if (gapParam->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
                ESP_LOGE(BT_CLIENT_TAG, "Scanning stop failed, Error: %s", esp_err_to_name(gapParam->scan_stop_cmpl.status));
            } else {
                ESP_LOGI(BT_CLIENT_TAG, "--------------------------------------");
                ESP_LOGI(BT_CLIENT_TAG, "| Scanning stop successful           |");
                ESP_LOGI(BT_CLIENT_TAG, "--------------------------------------");
            }
            break;

        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (gapParam->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(BT_CLIENT_TAG, "Advertising stop failed, Error: %s", esp_err_to_name(gapParam->adv_stop_cmpl.status));
            } else {
                ESP_LOGI(BT_CLIENT_TAG, "--------------------------------------");
                ESP_LOGI(BT_CLIENT_TAG, "| Advertising stop successful        |");
                ESP_LOGI(BT_CLIENT_TAG, "--------------------------------------");
            }
            break;

        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(BT_CLIENT_TAG, "Updating connection params");
            ESP_LOGI(BT_CLIENT_TAG, "--------------------------------------");
            ESP_LOGI(BT_CLIENT_TAG, "Status: %d", gapParam->update_conn_params.status);
            ESP_LOGI(BT_CLIENT_TAG, "Min interval: %d", gapParam->update_conn_params.min_int);
            ESP_LOGI(BT_CLIENT_TAG, "Max interval: %d", gapParam->update_conn_params.max_int);
            ESP_LOGI(BT_CLIENT_TAG, "Current interval: %d", gapParam->update_conn_params.conn_int);
            ESP_LOGI(BT_CLIENT_TAG, "Latency: %d", gapParam->update_conn_params.latency);
            ESP_LOGI(BT_CLIENT_TAG, "Timeout: %d", gapParam->update_conn_params.timeout);
            ESP_LOGI(BT_CLIENT_TAG, "--------------------------------------");
            break;
        default:
            break;
    }
}

void BleClient::_gattcProfileEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
    auto data = (esp_ble_gattc_cb_param_t *)param;
    esp_err_t mtu_ret;

    switch (event) {
        case ESP_GATTC_REG_EVT:
            ESP_LOGI(BT_CLIENT_TAG, "GATTC Register event received");
            _logIfThrowsError(esp_ble_gap_set_scan_params(&bleScanParams), "Setting of scan params failed");
        case ESP_GATTC_CONNECT_EVT:
            profileInstances[PROFILE_APP_IDX].conn_id = data->connect.conn_id;
            memcpy(profileInstances[PROFILE_APP_IDX].remote_bda, data->connect.remote_bda, sizeof(esp_bd_addr_t));

            ESP_LOGI(BT_CLIENT_TAG, "Remote Bluetooth Device Address:");
            esp_log_buffer_hex(BT_CLIENT_TAG, profileInstances[PROFILE_APP_IDX].remote_bda, sizeof(esp_bd_addr_t));

            _logIfThrowsError(esp_ble_gattc_send_mtu_req(gattc_if, data->connect.conn_id), "MTU request failed");
            break;

        case ESP_GATTC_OPEN_EVT:
            if (param->open.status != ESP_GATT_OK){
                ESP_LOGE(BT_CLIENT_TAG, "GATTC Open failed, Error: %d", data->open.status);
            } else {
                ESP_LOGI(BT_CLIENT_TAG, "GATTC opened successfully");
            }
            break;

        case ESP_GATTC_DIS_SRVC_CMPL_EVT:
            if (param->dis_srvc_cmpl.status != ESP_GATT_OK){
                ESP_LOGE(BT_CLIENT_TAG, "GATTC Service Discovery failed, Error: %d", param->dis_srvc_cmpl.status);
                break;
            }
            ESP_LOGI(BT_CLIENT_TAG, "Service Discovery completed successfully, Connection ID: %d", param->dis_srvc_cmpl.conn_id);
            esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
            break;

        case ESP_GATTC_CFG_MTU_EVT:
            if (param->cfg_mtu.status != ESP_GATT_OK){
                ESP_LOGE(BT_CLIENT_TAG, "MTU Configuration failed, Error: %x", param->cfg_mtu.status);
            } else {
                ESP_LOGI(BT_CLIENT_TAG, "MTU Configuration event successful, Status: %d, MTU: %d, Connection ID: %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
            }
            break;

        case ESP_GATTC_SEARCH_RES_EVT: {
            ESP_LOGI(BT_CLIENT_TAG, "SEARCH RES: conn_id = %x is primary service %d", data->search_res.conn_id, data->search_res.is_primary);
            ESP_LOGI(BT_CLIENT_TAG, "Start handle: %d, End handle: %d, Current handle: %d", data->search_res.start_handle, data->search_res.end_handle, data->search_res.srvc_id.inst_id);
            if (data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16 && data->search_res.srvc_id.uuid.uuid.uuid16 == REMOTE_SERVICE_UUID) {
                ESP_LOGI(BT_CLIENT_TAG, "Service found");
                _getServer = true;
                profileInstances[PROFILE_APP_IDX].service_start_handle = data->search_res.start_handle;
                profileInstances[PROFILE_APP_IDX].service_end_handle = data->search_res.end_handle;
                ESP_LOGI(BT_CLIENT_TAG, "Service UUID: %x", data->search_res.srvc_id.uuid.uuid.uuid16);
            }
            break;
        }

        case ESP_GATTC_SEARCH_CMPL_EVT:
            if (data->search_cmpl.status != ESP_GATT_OK){
                ESP_LOGE(BT_CLIENT_TAG, "Search service failed, Error: %x", data->search_cmpl.status);
                break;
            }
            if (data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE) {
                ESP_LOGI(BT_CLIENT_TAG, "Got service information from remote device");
            } else if (data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH) {
                ESP_LOGI(BT_CLIENT_TAG, "Got service information from flash");
            } else {
                ESP_LOGI(BT_CLIENT_TAG, "Unknown service information source");
            }
            ESP_LOGI(BT_CLIENT_TAG, "GATTC Search completed successfully, Connection ID: %d", data->search_cmpl.conn_id);

            if (_getServer) {
                uint16_t count = 0;
                esp_gatt_status_t status = esp_ble_gattc_get_attr_count(gattc_if, data->search_cmpl.conn_id,
                                                                        ESP_GATT_DB_CHARACTERISTIC,
                                                                        profileInstances[PROFILE_APP_IDX].service_start_handle,
                                                                        profileInstances[PROFILE_APP_IDX].service_end_handle,
                                                                        INVALID_HANDLE, &count);
                if (status != ESP_GATT_OK) {
                    ESP_LOGE(BT_CLIENT_TAG, "ESP GATTC Get Attribute Count characteristic failed");
                }

                if (count > 0){
                    charElemResult = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                    if (!charElemResult){
                        ESP_LOGI(BT_CLIENT_TAG, "Failed to allocate memory for result");

                    } else {
                        status = esp_ble_gattc_get_char_by_uuid(gattc_if,
                                                                data->search_cmpl.conn_id,
                                                                profileInstances[PROFILE_APP_IDX].service_start_handle,
                                                                profileInstances[PROFILE_APP_IDX].service_end_handle,
                                                                remote_filter_char_uuid,
                                                                charElemResult,
                                                                &count);
                        if (status != ESP_GATT_OK){
                            ESP_LOGE(BT_CLIENT_TAG, "ESP GATTC Get Characteristic failed");
                        }

                        if (count > 0 && (charElemResult[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)){
                            ESP_LOGI(BT_CLIENT_TAG, "Character count: %d", count);
                            profileInstances[PROFILE_APP_IDX].char_handle = charElemResult[0].char_handle;
                            esp_ble_gattc_register_for_notify (gattc_if, profileInstances[PROFILE_APP_IDX].remote_bda, charElemResult[0].char_handle);
                            esp_ble_gattc_read_char(gattc_if, data->search_cmpl.conn_id, profileInstances[PROFILE_APP_IDX].char_handle, ESP_GATT_AUTH_REQ_NONE);
                        }
                    }
                    /* free char_elem_result */
                    free(charElemResult);
                }else{
                    ESP_LOGE(BT_CLIENT_TAG, "no char found");
                }
            }
            break;

        case ESP_GATTC_READ_CHAR_EVT:
        {
            const char *char_pointer = (char *)data->read.value;
            int battery_percentage = (int) *char_pointer;
            ESP_LOGI(BT_CLIENT_TAG, "(p_data->read.value) Battery level: %d %%", battery_percentage);
            break;
        }


        case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
            ESP_LOGI(BT_CLIENT_TAG, "Registered for notify event");
            if (data->reg_for_notify.status != ESP_GATT_OK){
                ESP_LOGE(BT_CLIENT_TAG, "Register for notify failed: error status = %d", data->reg_for_notify.status);
            } else {
                uint16_t count = 0;
                uint16_t notify_en = 1;
                esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count( gattc_if,
                                                                             profileInstances[PROFILE_APP_IDX].conn_id,
                                                                             ESP_GATT_DB_DESCRIPTOR,
                                                                             profileInstances[PROFILE_APP_IDX].service_start_handle,
                                                                             profileInstances[PROFILE_APP_IDX].service_end_handle,
                                                                             profileInstances[PROFILE_APP_IDX].char_handle,
                                                                             &count);
                if (ret_status != ESP_GATT_OK){
                    ESP_LOGE(BT_CLIENT_TAG, "ESP GATTC Get Attribute Count error");
                }
                if (count > 0){
                    descrElemResult = static_cast<esp_gattc_descr_elem_t *>(malloc(sizeof(esp_gattc_descr_elem_t) * count));
                    if (!descrElemResult){
                        ESP_LOGE(BT_CLIENT_TAG, "malloc error, gattc no mem");
                    } else {
                        ret_status = esp_ble_gattc_get_descr_by_char_handle(gattc_if,
                                                                            profileInstances[PROFILE_APP_IDX].conn_id,
                                                                            data->reg_for_notify.handle,
                                                                            notify_descr_uuid,
                                                                            descrElemResult,
                                                                            &count);
                        if (ret_status != ESP_GATT_OK){
                            ESP_LOGE(BT_CLIENT_TAG, "esp_ble_gattc_get_descr_by_char_handle error");
                        }
                        /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                        if (count > 0 && descrElemResult[0].uuid.len == ESP_UUID_LEN_16 && descrElemResult[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG){
                            ret_status = static_cast<esp_gatt_status_t>(esp_ble_gattc_write_char_descr(gattc_if,
                                                                                                       profileInstances[PROFILE_APP_IDX].conn_id,
                                                                                                       descrElemResult[0].handle,
                                                                                                       sizeof(notify_en),
                                                                                                       (uint8_t *) &notify_en,
                                                                                                       ESP_GATT_WRITE_TYPE_RSP,
                                                                                                       ESP_GATT_AUTH_REQ_NONE));
                        }
                        if (ret_status != ESP_GATT_OK){
                            ESP_LOGE(BT_CLIENT_TAG, "esp_ble_gattc_write_char_descr error");
                        }
                        free(descrElemResult);
                    }
                }
                else {
                    ESP_LOGE(BT_CLIENT_TAG, "Descriptor not found");
                }
            }
            break;
        }

        case ESP_GATTC_NOTIFY_EVT:
            if (data->notify.is_notify){
                ESP_LOGI(BT_CLIENT_TAG, "ESP_GATTC_NOTIFY_EVT, receive notify value:");
            }else{
                ESP_LOGI(BT_CLIENT_TAG, "ESP_GATTC_NOTIFY_EVT, receive indicate value:");
            }
                    esp_log_buffer_hex(BT_CLIENT_TAG, data->notify.value, data->notify.value_len);
            break;

        case ESP_GATTC_WRITE_DESCR_EVT:
            if (data->write.status != ESP_GATT_OK){
                ESP_LOGE(BT_CLIENT_TAG, "write descr failed, error status = %x", data->write.status);
                break;
            }
            ESP_LOGI(BT_CLIENT_TAG, "write descr success ");
            uint8_t write_char_data[35];
            for (int i = 0; i < sizeof(write_char_data); ++i)
            {
                write_char_data[i] = i % 256;
            }
            esp_ble_gattc_write_char( gattc_if,
                                      profileInstances[PROFILE_APP_IDX].conn_id,
                                      profileInstances[PROFILE_APP_IDX].char_handle,
                                      sizeof(write_char_data),
                                      write_char_data,
                                      ESP_GATT_WRITE_TYPE_RSP,
                                      ESP_GATT_AUTH_REQ_NONE);
            break;

        case ESP_GATTC_SRVC_CHG_EVT: {
            esp_bd_addr_t bda;
            memcpy(bda, data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
            ESP_LOGI(BT_CLIENT_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
                    esp_log_buffer_hex(BT_CLIENT_TAG, bda, sizeof(esp_bd_addr_t));
            break;
        }

        case ESP_GATTC_WRITE_CHAR_EVT:
            if (data->write.status != ESP_GATT_OK){
                ESP_LOGE(BT_CLIENT_TAG, "write char failed, error status = %x", data->write.status);
                break;
            }
            ESP_LOGI(BT_CLIENT_TAG, "write char success ");
            break;

        case ESP_GATTC_DISCONNECT_EVT:
            _isConnected = false;
            _getServer = false;
            ESP_LOGI(BT_CLIENT_TAG, "Bluetooth disconnected, Reason: %d", data->disconnect.reason);
            break;
        default:
            break;
    }
}

void BleClient::_gattcEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_interface_type, esp_ble_gattc_cb_param_t *gattcParam) {
    if (event == ESP_GATTC_REG_EVT) {
        if (gattcParam->reg.status == ESP_GATT_OK) {
            profileInstances[gattcParam->reg.app_id].gattc_if = gattc_interface_type;
            ESP_LOGI(BT_CLIENT_TAG, "App registratin successful. App ID: %04x, Interface type: %d", gattcParam->reg.app_id, gattc_interface_type);
        }
        else {
            ESP_LOGE(BT_CLIENT_TAG, "App registration failed, App ID: %04x, Status: %d", gattcParam->reg.app_id, gattcParam->reg.status);
            return;
        }
    }

    for (auto & profile: profileInstances) {
        if (profile.gattc_if == ESP_GATT_IF_NONE || profile.gattc_if == gattc_interface_type) {
            if (profile.gattc_cb) {
                profile.gattc_cb(event, gattc_interface_type, gattcParam);
            }
        }
    }
}

bool BleClient::_logIfThrowsError(esp_err_t error_code, const char* message, bool error_string)
{
    if (error_code) {
        if (error_string)
            ESP_LOGE(BT_CLIENT_TAG, "%s. Error: %s", message, esp_err_to_name(error_code));
        else
            ESP_LOGE(BT_CLIENT_TAG, "%s. Error: 0x%x", message, error_code);
        return true;
    }
    return false;
}
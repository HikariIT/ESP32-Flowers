#include "wifi_handler.hpp"

EventGroupHandle_t WifiHandler::_wifiEventGroup = xEventGroupCreate();
const char* WifiHandler::TAG = "WifiHandler";
int WifiHandler::_connRetryCount = 1;
bool WifiHandler::connected = false;


void WifiHandler::_handleWifiStationStarted(void* arg, esp_event_base_t espEventBase, int32_t eventId, void* eventData) {
    ESP_LOGI(WifiHandler::TAG, "Connecting to Access Point (%s)", WIFI_SSID);
    esp_wifi_connect();
}

void WifiHandler::_handleWifiStationDisconnected(void* arg, esp_event_base_t espEventBase, int32_t eventId, void* eventData) {
    if (WifiHandler::connected) {
        ESP_LOGI(WifiHandler::TAG, "Disconnected from Access Point (%s)", WIFI_SSID);
        WifiHandler::_connRetryCount = 1;
        WifiHandler::connected = false;
    }

    if (WifiHandler::_connRetryCount < MAX_CONN_RETRY_COUNT) {
        ESP_LOGI(WifiHandler::TAG, "Retrying connection to Access Point (%s)", WIFI_SSID);
        ESP_LOGI(WifiHandler::TAG, "Retry count: %d / %d", WifiHandler::_connRetryCount, MAX_CONN_RETRY_COUNT);
        WifiHandler::_connRetryCount++;
        esp_wifi_connect();
    } else {
        ESP_LOGI(WifiHandler::TAG, "Failed to connect to Access Point (%s)", WIFI_SSID);
        xEventGroupSetBits(WifiHandler::_wifiEventGroup, WIFI_FAIL_BIT);
    }
}

void WifiHandler::_handleIpReceived(void* arg, esp_event_base_t espEventBase, int32_t eventId, void* eventData) {
    auto ip_event = (ip_event_got_ip_t*) eventData;
    ESP_LOGI(WifiHandler::TAG, "Connected to Access Point (%s)", WIFI_SSID);
    ESP_LOGI(WifiHandler::TAG, "Received IP via DHCP: " IPSTR, IP2STR(&ip_event->ip_info.ip));
    xEventGroupSetBits(WifiHandler::_wifiEventGroup, WIFI_CONNECTED_BIT);
    WifiHandler::connected = true;
}

void WifiHandler::initializeWifiStation() {
    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("esp_netif_handlers", ESP_LOG_WARN);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiInitConfig));

    esp_event_handler_instance_t wifi_start_hdl, wifi_disconnect_hdl, ip_received_hdl;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_START,
                                                        &_handleWifiStationStarted, nullptr,
                                                        &wifi_start_hdl));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
                                                        &_handleWifiStationDisconnected, nullptr,
                                                        &wifi_disconnect_hdl));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &_handleIpReceived, nullptr,
                                                        &ip_received_hdl));

    wifi_config_t wifiConfig = {
        .sta = {
            .ssid               = WIFI_SSID,
            .password           = WIFI_PASS,
            .threshold          = {
                    .authmode   = WIFI_AUTH_WPA2_PSK,
            },
            .sae_pwe_h2e        = WPA3_SAE_PWE_BOTH,
            .sae_h2e_identifier = CONFIG_ESP_WIFI_PW_ID
        }
    };

    ESP_LOGI(WifiHandler::TAG, "Setting WiFi configuration SSID: %s", wifiConfig.sta.ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WifiHandler::TAG, "Waiting for WiFi connection...");

    auto bits = xEventGroupWaitBits(_wifiEventGroup,
                                    WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                    pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(WifiHandler::TAG, "Connected to Access Point (%s)", WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(WifiHandler::TAG, "Failed to connect to Access Point (%s)", WIFI_SSID);
    } else {
        ESP_LOGE(WifiHandler::TAG, "UNEXPECTED EVENT");
    }
}
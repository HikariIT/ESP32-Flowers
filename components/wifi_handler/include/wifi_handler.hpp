#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_log.h>

#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1
#define MAX_CONN_RETRY_COUNT    1000

#define WIFI_SSID               "Hikari"
#define WIFI_PASS               "lubieplacki"

class WifiHandler {
private:
    static const char* TAG;

    static int _connRetryCount;
    static EventGroupHandle_t _wifiEventGroup;

    static void _handleWifiStationStarted(void* arg, esp_event_base_t espEventBase, int32_t eventId, void* eventData);
    static void _handleWifiStationDisconnected(void* arg, esp_event_base_t espEventBase, int32_t eventId, void* eventData);
    static void _handleIpReceived(void* arg, esp_event_base_t espEventBase, int32_t eventId, void* eventData);

public:
    static bool connected;
    static void initializeWifiStation();
};


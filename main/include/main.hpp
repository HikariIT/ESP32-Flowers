#pragma once

#include <nvs_flash.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "main.hpp"
#include "led_handler.hpp"
#include "wifi_handler.hpp"
#include "http_handler.hpp"
#include "ble_server.hpp"
#include "ble_client.hpp"
#include "mqtt_handler.hpp"

class App final {
public:
    static void init();
};

extern "C" void app_main();
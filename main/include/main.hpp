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
#include "monitor_handler.hpp"
#include "thermometer_handler.hpp"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_mac.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (5) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (800) // Frequency in Hertz. Set frequency at 4 kHz

#define A 220
#define B 247
#define C 261
#define D 294
#define E 329
#define F 349
#define G 391

class App final {
public:
    static void init();
    static void initializeId();
};

extern "C" void app_main();
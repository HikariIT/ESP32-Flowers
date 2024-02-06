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
#include "button_handler.hpp"
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (5) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (800) // Frequency in Hertz. Set frequency at 4 kHz

#define h 494
#define C 523
#define D 587
#define E 659
#define F 698
#define Fis 740
#define G 784
#define A 880
#define H 988




class App final {
public:
    static void init();
};

extern "C" void app_main();
#pragma once

#include <thread>
#include <functional>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "led_handler.hpp"
#include "monitor_handler.hpp"
#include <esp_log.h>

#define PUSH_BUTTON_PIN GPIO_NUM_33
class ButtonHandler final {
    static const char *TAG;
public:
    static void buttonInit();
    static void ButtonHandlerTask(void *pvParameters);
    static void buttonClicked();
};

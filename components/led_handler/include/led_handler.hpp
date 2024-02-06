#pragma once

#include <thread>
#include <functional>

#include <esp_log.h>
#include "wifi_handler.hpp"


#include "gpio_cxx.hpp"

#define LED_GPIO_PORT 2

class LedHandler final {
private:
    static idf::GPIOLevel _level;
    static const idf::GPIO_Output *_led;

public:
    static const char *TAG;
    static const int BLINK_PERIOD_MS;

    static void setLevel(idf::GPIOLevel level);
    static void toggleLevel();
    static void ledHandlerTask(void *pvParameters);
};

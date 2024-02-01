#pragma once

#include <thread>
#include <functional>

#include <esp_log.h>
#include "wifi_handler.hpp"
#include "gpio_cxx.hpp"

#define LED_GPIO_PORT 27

enum TemperatureStatus {
    NORMAL,
    HIGH,
    LOW,
};

enum HumidityStatus {
    H_NORMAL,
    H_HIGH,
    H_LOW,
};

struct DHT11Conditions {
    TemperatureStatus temperatureStatus;
    HumidityStatus humidityStatus;
};

class LedHandler final {
private:
    static int current_temperature;
    static int current_humidity;
    static int min_temperature;
    static int max_temperature;
    static int min_humidity;
    static int max_humidity;
    static idf::GPIOLevel _level;
    static const idf::GPIO_Output *_led;

public:
    static const char *TAG;
    static const int BLINK_PERIOD_MS;
    static void setData(int temperature, int humidity);
    static void setLevel(idf::GPIOLevel level);
    static void toggleLevel();
    static void ledHandlerTask(void *pvParameters);
    static DHT11Conditions checkConditions();
};


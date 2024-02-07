#pragma once


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
#include "monitor_handler.hpp"
#include "diode_handler.hpp"
#include "speaker_handler.hpp"
#include "soil_moisture_handler.hpp"
#define REFRESH_RATE 10000
#ifndef DHT11_H_
#define DHT11_H_
#endif
#define DHT11_GPIO GPIO_NUM_32

enum dht11_status {
    DHT11_CRC_ERROR = -2,
    DHT11_TIMEOUT_ERROR,
    DHT11_OK
};

struct dht11_reading {
    int status;
    int temperature;
    int humidity;
};


class ThermometerHandler {
    const static char *TAG;
    static gpio_num_t dht_gpio;
    static struct dht11_reading last_read;
    static int64_t last_read_time;

    private:
        static int _waitOrTimeout(uint16_t microSeconds, int level);
        static int _checkCRC(uint8_t data[]);
        static void _sendStartSignal();
        static int _checkResponse();
        static dht11_reading _timeoutError();
        static dht11_reading _crcError();
        static void DHT11_init(gpio_num_t gpio_num);
        static dht11_reading DHT11_read();

        
    public:
        static void initializeTermometer();
        static void ThermometerHandlerTask(void *pvParameter);
};

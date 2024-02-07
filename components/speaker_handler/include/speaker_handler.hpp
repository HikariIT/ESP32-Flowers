#pragma once


#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/ledc.h"
#include "math.h"
#define A 220
#define B 247
#define C 261
#define D 294
#define E 329
#define F 349
#define G 391
#define c 523
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (5) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (800) // Frequency in Hertz. Set frequency at 4 kHz
struct Note {
    int note;
    int duration;
};

class SpeakerHandler final {
    static const char *TAG;
    static int min_temp;
    static int max_temp;
    static int min_humidity;
    static int max_humidity;
    static int current_humidity;
    static int current_temperature;
    public:
        static void init_speaker();
        static void speaker_task(void *pvParameter);
        static void update_data(int temperature, int humidity);
    private:
        static void ledc_init();
        static void play_melody(Note *melody, int melody_size, int tempo);
        static void play_startup();
        static Note startup_sound[4];
        static void update_limits(int min_temp, int max_temp, int min_humidity, int max_humidity);
};
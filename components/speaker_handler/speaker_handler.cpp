#include "speaker_handler.hpp"

const char *SpeakerHandler::TAG = "SpeakerHandler";

Note SpeakerHandler::startup_sound[4] = {
    {c, 1},
    {G, 1},
    {E, 1},
    {C, 3}
};


int SpeakerHandler::min_temp = 10;
int SpeakerHandler::max_temp = 40;
int SpeakerHandler::min_humidity = 10;
int SpeakerHandler::max_humidity = 70;
int SpeakerHandler::current_humidity = 20;
int SpeakerHandler::current_temperature = 20;
void SpeakerHandler::init_speaker() {
    SpeakerHandler::ledc_init();
    SpeakerHandler::play_startup();
}

void SpeakerHandler::update_limits(int min_temp, int max_temp, int min_humidity, int max_humidity) {
    SpeakerHandler::min_temp = min_temp;
    SpeakerHandler::max_temp = max_temp;
    SpeakerHandler::min_humidity = min_humidity;
    SpeakerHandler::max_humidity = max_humidity;
}
void alarm(){
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    for(int i=0; i<100; i++){
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, sin(i*3.14/100) * 1000 + 100);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}
void SpeakerHandler::speaker_task(void *pvParameter) {

    
    while (1) {

        if(SpeakerHandler::current_temperature < SpeakerHandler::min_temp){
            alarm();
        }
        else if(SpeakerHandler::current_temperature > SpeakerHandler::max_temp){
            alarm();
        }
        else if(SpeakerHandler::current_humidity < SpeakerHandler::min_humidity){
            alarm();
        }
        else if(SpeakerHandler::current_humidity > SpeakerHandler::max_humidity){
            alarm();
        }
        else{
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }



    }
}
void SpeakerHandler::update_data(int temperature, int humidity) {
    SpeakerHandler::current_temperature = temperature;
    SpeakerHandler::current_humidity = humidity;
}

void SpeakerHandler::ledc_init() {
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = LEDC_OUTPUT_IO,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void SpeakerHandler::play_startup() {
    SpeakerHandler::play_melody(startup_sound, 4, 350);
}

void SpeakerHandler::play_melody(Note *melody, int melody_size, int tempo) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    for(int i=0; i<melody_size; i++){
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, melody[i].note);
        ESP_LOGI("App", "Note: %d", melody[i].note);
        vTaskDelay(melody[i].duration * tempo / portTICK_PERIOD_MS);

    }
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}
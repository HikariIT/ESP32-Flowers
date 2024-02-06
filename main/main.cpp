#include "main.hpp"
//#include <driver/dac_common.h>
void App::init() {
    ESP_ERROR_CHECK(nvs_flash_init());



    xTaskCreate(&LedHandler::ledHandlerTask, "led_handler", 2048,
                nullptr, 4, nullptr);
    // BleServer::initializeBluetoothServer();
    MonitorHandler::initializeMonitor();
    BleClient::initializeBluetoothClient();
    
    ThermometerHandler::initializeTermometer();
    xTaskCreate(&ThermometerHandler::ThermometerHandlerTask, "thermometer_handler", 2048,
                nullptr, 4, nullptr);
    ESP_LOGI("App", "Initialized");
    
    WifiHandler::initializeWifiStation();
    // MqttHandler::initializeMqttClient();

    //xTaskCreate(&HttpHandler::httpHandlerTask, "http_handler", 2048,
    //            nullptr, 5, nullptr);
    ButtonHandler::buttonInit();
    xTaskCreate(&ButtonHandler::ButtonHandlerTask, "button_handler", 2048,
                nullptr, 5, nullptr);
}
static void example_ledc_init(void)
{
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

struct Note {
    int note;
    int duration;
};
extern "C" void app_main() {
    Note internationale[17] = {
        {D, 2},
        {G, 3},

        {Fis, 1},
        {A, 1},
        {G, 1},
        {D, 1},
        {h, 1},

        {E, 4},
        {C, 3},
        {E, 1},
        {A, 3},
        {G, 1},

        {Fis, 1},
        {E, 1},
        {D ,1},
        {C, 1},
        {h, 6}
    };
    /*
    Note tetris[50] = {
        {E, 2},
        {H, 1},
        {C, 1},
        {D, 2},
        {C, 1},
        {B, 1},
        {A, 2},
        {A, 1},
        {C, 1},
        {E, 2},

        {D, 1},
        {C, 1},
        {H, 2},
        {H, 1},
        {C, 1},
        {D, 2},
        {E, 1},
        {C, 1},
        {A, 2},
        {A, 1},

        {D, 1},
        {F, 2},
        {A, 1},
        {G, 1},
        {F, 4},
        {E, 2},
        {C, 1},
        {E, 1},
        {D, 2},
        {C, 1},

        {B, 1},
        {A, 2},
        {A, 1},
        {C, 1},
        {E, 2},
        {D, 1},
        {C, 1},
        {B, 2},
        {B, 1},
        {C, 1},

        {D, 2},
        {E, 1},
        {C, 1},
        {A, 2},
        {A, 1},
        {D, 1},
        {F, 2},
        {A, 1},
        {G, 1},
        {F, 4}
    };
    */
    Note notes[18] = {
        {G, 2},
        {E, 2},
        {E, 2},
        {F, 2},
        {D, 2},

        {D, 2},
        {C, 1},
        {E, 1},
        {G, 4},
        {G, 2},

        {E, 2},
        {E, 2},
        {F, 2},
        {D, 2},
        {D, 2},

        {C, 1},
        {E, 1},
        {C, 4}
    };
    App helloApp;
    helloApp.init();
        // Konfiguracja timera PWM


    MonitorHandler::destroyCapitalism();
    example_ledc_init();
    
    //ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
    // Set duty to 50%
    //ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    // Update duty to apply the new value
    /*
    for(int i=0; i<17; i++){
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, internationale[i].note);
        ESP_LOGI("App", "Note: %d", internationale[i].note);
        vTaskDelay(internationale[i].duration * 300 / portTICK_PERIOD_MS);

    }
    */

}


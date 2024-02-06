#include "main.hpp"

void App::init() {
    initializeId();

    xTaskCreate(&LedHandler::ledHandlerTask, "led_handler", 2048,
                nullptr, 4, nullptr);

    // MonitorHandler::initializeMonitor();

    // ThermometerHandler::initializeTermometer();
    /*xTaskCreate(&ThermometerHandler::ThermometerHandlerTask, "thermometer_handler", 2048,
                nullptr, 4, nullptr);
    ESP_LOGI("App", "Initialized");*/
    // BleClient::initializeBluetoothClient();

    BleServer::initializeBluetoothServer();
    // BleClient::initializeBluetoothClient();

    WifiHandler::initializeWifiStation();
    // MqttHandler::initializeMqttClient();

    //xTaskCreate(&HttpHandler::httpHandlerTask, "http_handler", 2048,
    //            nullptr, 5, nullptr);

    /*
    auto response = HttpHandler::makeGetRequest("Hello, World!", "192.168.0.150", "/", "3000");
    ESP_LOGI("App", "Response: %s", response->body);
    ESP_LOGI("App", "Response code: %d", response->status_code);*/

    nvs_handle_t nvsHandler;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvsHandler));

    // Get the device ID
    size_t deviceNameSize = 13;
    char* deviceName = (char*) malloc(deviceNameSize);
    auto nvs_error_code = nvs_get_str(nvsHandler, "deviceId", deviceName, &deviceNameSize);
    const char* mockToken = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VySWQiOiJlMTQxMmZycjQxMjVkZXMzIn0.OF7FnAuuPfmmz93Kw_VUHFLCfWVXZdYcKiD4o6724S8";

    std::string jsonBody = R"({"token": ")";
    jsonBody += mockToken;
    jsonBody += R"(", "deviceId": ")";
    jsonBody += deviceName;
    jsonBody += R"("})";

    auto response2 = HttpHandler::makePostRequest("Hello, World!", "192.168.0.150", "/", "3000",
                                                    jsonBody.c_str());
    ESP_LOGI("App", "Response: %s", response2->body);
    ESP_LOGI("App", "Response code: %d", response2->status_code);
}

void App::initializeId() {
    ESP_ERROR_CHECK(nvs_flash_init());

    uint8_t chipId[6];
    esp_efuse_mac_get_default(chipId);
    char chipIdString[13];
    sprintf(chipIdString, "%02X%02X%02X%02X%02X%02X", chipId[0], chipId[1],
            chipId[2], chipId[3], chipId[4], chipId[5]);

    nvs_handle_t nvsHandler;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvsHandler));

    nvs_set_str(nvsHandler, "deviceId", chipIdString);
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
    Note tetris[50] = {
        {E, 2},
        {B, 1},
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

    example_ledc_init();
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
    // Set duty to 50%
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    // Update duty to apply the new value

    for(int i=0; i<18; i++){
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, tetris[i].note);
        ESP_LOGI("App", "Note: %d", tetris[i].note);
        vTaskDelay(tetris[i].duration * 30 - 20 / portTICK_PERIOD_MS);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(50 / portTICK_PERIOD_MS);

    }
}


#include "button_handler.hpp"

const char *ButtonHandler::TAG = "ButtonHandler";


void ButtonHandler::buttonInit() {
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
}

void ButtonHandler::ButtonHandlerTask(void *pvParameters) {
    int64_t last_read_time = 0;
    while (true) {
        if (gpio_get_level(PUSH_BUTTON_PIN) == 1) {
            last_read_time = esp_timer_get_time();
            buttonClicked();
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            if (gpio_get_level(PUSH_BUTTON_PIN) == 1) {
                ESP_LOGI(TAG, "Long press detected");
                MonitorHandler::destroyCapitalism();
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void ButtonHandler::buttonClicked() {
    //TODO: Implement button click
    ESP_LOGI(TAG, "Button clicked");
    LedHandler::toggleLevel();
}
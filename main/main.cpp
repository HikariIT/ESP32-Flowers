#include "main.hpp"

void App::init() {
    ESP_ERROR_CHECK(nvs_flash_init());

    xTaskCreate(&LedHandler::ledHandlerTask, "led_handler", 2048,
                nullptr, 4, nullptr);

    WifiHandler::initializeWifiStation();

    xTaskCreate(&HttpHandler::httpHandlerTask, "http_handler", 2048,
                nullptr, 5, nullptr);
}

extern "C" void app_main() {
    App helloApp;
    helloApp.init();
}


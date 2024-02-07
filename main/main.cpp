#include "main.hpp"



void App::init() {
    //initializeId();

    xTaskCreate(&LedHandler::ledHandlerTask, "led_handler", 2048,
                nullptr, 4, nullptr);
    xTaskCreate(&DiodeHandler::DiodeHandlerTask, "diode_handler", 2048,
                nullptr, 4, nullptr);
    // BleServer::initializeBluetoothServer();
    MonitorHandler::initializeMonitor();
    //BleClient::initializeBluetoothClient();
    SpeakerHandler::init_speaker();
    xTaskCreate(&SpeakerHandler::speaker_task, "speaker_task", 2048,
                nullptr, 4, nullptr);
    ThermometerHandler::initializeTermometer();
    xTaskCreate(&ThermometerHandler::ThermometerHandlerTask, "thermometer_handler", 2048,
                nullptr, 4, nullptr);
    ESP_LOGI("App", "Initialized");
    // WifiHandler::initializeWifiStation();
    // MqttHandler::initializeMqttClient();

    //xTaskCreate(&HttpHandler::httpHandlerTask, "http_handler", 2048,
    //            nullptr, 5, nullptr);

    SoilMoistureHandler::SoilMoistureInit();
}




extern "C" void app_main() {

    
    App helloApp;
    helloApp.init();
        // Konfiguracja timera PWM

}


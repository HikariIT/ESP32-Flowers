#include "led_handler.hpp"

const idf::GPIO_Output *LedHandler::_led = new idf::GPIO_Output(idf::GPIONum(LED_GPIO_PORT));
idf::GPIOLevel LedHandler::_level = idf::GPIOLevel::LOW;
const int LedHandler::BLINK_PERIOD_MS = 500;
const char *LedHandler::TAG = "LedHandler";

void LedHandler::setLevel(idf::GPIOLevel level) {
    _level = level;
    if (_level == idf::GPIOLevel::HIGH)
        _led->set_high();
    else
        _led->set_low();
}

void LedHandler::toggleLevel() {
    setLevel(_level == idf::GPIOLevel::HIGH ? idf::GPIOLevel::LOW : idf::GPIOLevel::HIGH);
}

void LedHandler::ledHandlerTask(void *pvParameters) {
    ESP_LOGI("ledHandlerTask", "Starting task: Led Handler");

    for (;;) {
        if (WifiHandler::connected)
            setLevel(idf::GPIOLevel::LOW);
        else
            toggleLevel();
        std::this_thread::sleep_for(std::chrono::milliseconds(BLINK_PERIOD_MS));
    }
}
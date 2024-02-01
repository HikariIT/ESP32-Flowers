#include "led_handler.hpp"


int LedHandler::current_temperature = 0;
int LedHandler::current_humidity = 0;
int LedHandler::min_temperature = 21;
int LedHandler::max_temperature = 22;
int LedHandler::min_humidity = 37;
int LedHandler::max_humidity = 38;

const idf::GPIO_Output *LedHandler::_led = new idf::GPIO_Output(idf::GPIONum(LED_GPIO_PORT));
idf::GPIOLevel LedHandler::_level = idf::GPIOLevel::LOW;
const int LedHandler::BLINK_PERIOD_MS = 250;
const char *LedHandler::TAG = "LedHandler";
void LedHandler::setData(int temperature, int humidity) {
    LedHandler::current_temperature = temperature;
    LedHandler::current_humidity = humidity;
}
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

DHT11Conditions LedHandler::checkConditions(){
    DHT11Conditions conditions;

    if (LedHandler::current_temperature < LedHandler::min_temperature)
        conditions.temperatureStatus = TemperatureStatus::LOW;
    else if(LedHandler::current_temperature > LedHandler::max_temperature)
        conditions.temperatureStatus = TemperatureStatus::HIGH;
    else
        conditions.temperatureStatus = TemperatureStatus::NORMAL;

    if(LedHandler::current_humidity < LedHandler::min_humidity)
        conditions.humidityStatus = HumidityStatus::H_LOW;
    else if(LedHandler::current_humidity > LedHandler::max_humidity)
        conditions.humidityStatus = HumidityStatus::H_HIGH;
    else
        conditions.humidityStatus = HumidityStatus::H_NORMAL;

    return conditions;
}

void LedHandler::ledHandlerTask(void *pvParameters) {
    ESP_LOGI("ledHandlerTask", "Starting task: Led Handler");

    for (;;) {
        DHT11Conditions conditions = LedHandler::checkConditions();
        if (conditions.temperatureStatus == TemperatureStatus::NORMAL && conditions.humidityStatus == HumidityStatus::H_NORMAL)
            setLevel(idf::GPIOLevel::LOW);
        else
            toggleLevel();

        std::this_thread::sleep_for(std::chrono::milliseconds(BLINK_PERIOD_MS));
    }
}
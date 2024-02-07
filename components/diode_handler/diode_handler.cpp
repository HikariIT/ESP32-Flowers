#include "diode_handler.hpp"


int DiodeHandler::current_temperature = 0;
int DiodeHandler::current_humidity = 0;
int DiodeHandler::min_temperature = 15;
int DiodeHandler::max_temperature = 26;
int DiodeHandler::min_humidity = 25;
int DiodeHandler::max_humidity = 35;

const idf::GPIO_Output *DiodeHandler::_led = new idf::GPIO_Output(idf::GPIONum(DIODE_GPIO_PORT));
idf::GPIOLevel DiodeHandler::_level = idf::GPIOLevel::LOW;
const int DiodeHandler::BLINK_PERIOD_MS = 250;
const char *DiodeHandler::TAG = "DiodeHandler";

void DiodeHandler::setData(int temperature, int humidity) {
    DiodeHandler::current_temperature = temperature;
    DiodeHandler::current_humidity = humidity;
}
void DiodeHandler::setLevel(idf::GPIOLevel level) {
    _level = level;
    if (_level == idf::GPIOLevel::HIGH)
        _led->set_high();
    else
        _led->set_low();
}

void DiodeHandler::toggleLevel() {
    setLevel(_level == idf::GPIOLevel::HIGH ? idf::GPIOLevel::LOW : idf::GPIOLevel::HIGH);
}

DHT11Conditions DiodeHandler::checkConditions(){
    DHT11Conditions conditions;

    if (DiodeHandler::current_temperature < DiodeHandler::min_temperature)
        conditions.temperatureStatus = TemperatureStatus::LOW;
    else if(DiodeHandler::current_temperature > DiodeHandler::max_temperature)
        conditions.temperatureStatus = TemperatureStatus::HIGH;
    else
        conditions.temperatureStatus = TemperatureStatus::NORMAL;

    if(DiodeHandler::current_humidity < DiodeHandler::min_humidity)
        conditions.humidityStatus = HumidityStatus::H_LOW;
    else if(DiodeHandler::current_humidity > DiodeHandler::max_humidity)
        conditions.humidityStatus = HumidityStatus::H_HIGH;
    else
        conditions.humidityStatus = HumidityStatus::H_NORMAL;

    return conditions;
}

void DiodeHandler::DiodeHandlerTask(void *pvParameters) {
    ESP_LOGI("DiodeHandlerTask", "Starting task: Led Handler");

    for (;;) {
        DHT11Conditions conditions = DiodeHandler::checkConditions();
        if (conditions.temperatureStatus == TemperatureStatus::NORMAL && conditions.humidityStatus == HumidityStatus::H_NORMAL)
            setLevel(idf::GPIOLevel::LOW);
        else
            toggleLevel();

        std::this_thread::sleep_for(std::chrono::milliseconds(BLINK_PERIOD_MS));
    }
}
#include "thermometer_handler.hpp"

//#include "monitor_handler.hpp"
const char* ThermometerHandler::TAG = "ThermometerHandler";
gpio_num_t ThermometerHandler::dht_gpio = DHT11_GPIO;
struct dht11_reading ThermometerHandler::last_read = {DHT11_OK, -1, -1};
int64_t ThermometerHandler::last_read_time = -2000000;
int ThermometerHandler::_waitOrTimeout(uint16_t microSeconds, int level) {
    int micros_ticks = 0;
    while(gpio_get_level(ThermometerHandler::dht_gpio) == level) { 
        if(micros_ticks++ > microSeconds) 
            return DHT11_TIMEOUT_ERROR;
        ets_delay_us(1);
    }
    return micros_ticks;
}

int ThermometerHandler::_checkCRC(uint8_t data[]) {
    if(data[4] == (data[0] + data[1] + data[2] + data[3]))
        return DHT11_OK;
    else
        return DHT11_CRC_ERROR;
}

void ThermometerHandler::_sendStartSignal() {
    gpio_set_direction(ThermometerHandler::dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(ThermometerHandler::dht_gpio, 0);
    ets_delay_us(20 * 1000);
    gpio_set_level(ThermometerHandler::dht_gpio, 1);
    ets_delay_us(40);
    gpio_set_direction(ThermometerHandler::dht_gpio, GPIO_MODE_INPUT);
}

int ThermometerHandler::_checkResponse() {
    /* Wait for next step ~80us*/
    if(ThermometerHandler::_waitOrTimeout(80, 0) == DHT11_TIMEOUT_ERROR)
        return DHT11_TIMEOUT_ERROR;

    /* Wait for next step ~80us*/
    if(ThermometerHandler::_waitOrTimeout(80, 1) == DHT11_TIMEOUT_ERROR) 
        return DHT11_TIMEOUT_ERROR;

    return DHT11_OK;
}

struct dht11_reading ThermometerHandler::_timeoutError() {
    struct dht11_reading timeoutError = {DHT11_TIMEOUT_ERROR, -1, -1};
    return timeoutError;
}

struct dht11_reading ThermometerHandler::_crcError() {
    struct dht11_reading crcError = {DHT11_CRC_ERROR, -1, -1};
    return crcError;
}

void ThermometerHandler::DHT11_init(gpio_num_t gpio_num) {
    /* Wait 1 seconds to make the device pass its initial unstable status */
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ThermometerHandler::dht_gpio = gpio_num;
}

struct dht11_reading ThermometerHandler::DHT11_read() {
    /* Tried to sense too son since last read (dht11 needs ~2 seconds to make a new read) */
    if(esp_timer_get_time() - 2000000 < ThermometerHandler::last_read_time) {
        return ThermometerHandler::last_read;
    }

    ThermometerHandler::last_read_time = esp_timer_get_time();

    uint8_t data[5] = {0,0,0,0,0};

    _sendStartSignal();

    if(_checkResponse() == DHT11_TIMEOUT_ERROR)
        return ThermometerHandler::last_read = _timeoutError();
    
    /* Read response */
    for(int i = 0; i < 40; i++) {
        /* Initial data */
        if(ThermometerHandler::_waitOrTimeout(50, 0) == DHT11_TIMEOUT_ERROR)
            return ThermometerHandler::last_read = _timeoutError();
                
        if(ThermometerHandler::_waitOrTimeout(70, 1) > 28) {
            /* Bit received was a 1 */
            data[i/8] |= (1 << (7-(i%8)));
        }
    }

    if(_checkCRC(data) != DHT11_CRC_ERROR) {
        ThermometerHandler::last_read.status = DHT11_OK;
        ThermometerHandler::last_read.temperature = data[2];
        ThermometerHandler::last_read.humidity = data[0];
        return ThermometerHandler::last_read;
    } else {
        return ThermometerHandler::last_read = _crcError();
    }
}

void ThermometerHandler::initializeTermometer() {
    DHT11_init(DHT11_GPIO);
}

void ThermometerHandler::ThermometerHandlerTask(void *pvParameter) {
    struct dht11_reading reading;
    while(1) {
        reading = DHT11_read();
        if(reading.status == DHT11_OK) {
            ESP_LOGI(TAG, "Temperature: %d*C Humidity: %d%%", reading.temperature, reading.humidity);
            MonitorHandler::updateData(reading.temperature, reading.humidity);
            LedHandler::setData(reading.temperature, reading.humidity);
            DHT11Conditions conditions = LedHandler::checkConditions();
            if (conditions.temperatureStatus == TemperatureStatus::HIGH) 
                MonitorHandler::updateCommunicate(" Temp too high!", 0);
            else if (conditions.temperatureStatus == TemperatureStatus::LOW)
                MonitorHandler::updateCommunicate(" Temp too low!", 0);

            if (conditions.humidityStatus == HumidityStatus::H_HIGH)
                MonitorHandler::updateCommunicate(" Hum. too high!", 0);
            else if (conditions.humidityStatus == HumidityStatus::H_LOW)
                MonitorHandler::updateCommunicate(" Hum. too low!", 0);

        } else {
            ESP_LOGE(TAG, "Could not read data from sensor");
            MonitorHandler::updateData(-1, -1);
            MonitorHandler::updateCommunicate(" Could not \n read data \n from sensor", 1);
        }
        vTaskDelay(REFRESH_RATE / portTICK_PERIOD_MS);
    }
}
#include "soil_moisture_handler.hpp"

static const char *TAG = "MoistureSensor";

void SoilMoistureHandler::SoilMoistureInit() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
}

float SoilMoistureHandler::read_soil_moisture() {
    int raw_value = adc1_get_raw(ADC1_CHANNEL_6);
    float moisture = 100 - (raw_value / 4095.0) * 100.0; // Konwersja na procenty wilgotności
    float voltage = (raw_value / 4095.0) * 3.3;
    return moisture;
}
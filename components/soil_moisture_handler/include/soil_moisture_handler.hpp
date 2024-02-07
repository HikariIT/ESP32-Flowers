#ifndef SOLID_MOISTURE_SENSOR_H  
#define SOLID_MOISTURE_SENSOR_H  
#include "esp_system.h"
#include "esp_log.h"
#include "driver/adc.h"

#define MOISTURE_SENSOR_PIN 34
class SoilMoistureHandler { 
    public:
        static void SoilMoistureInit();
        static float read_soil_moisture();


};
#endif


#include <cstdio>
#include <cstdint>
#include <cstring>
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"


#include "esp_log.h"
#include "mqtt_client.h"

#define CONFIG_BROKER_URI "mqtt://192.168.91.97:1883"

class MqttHandler {
    static const char* TAG;
public:
    static void initializeMqttClient();
private:
    static void _publish(esp_mqtt_client_handle_t client, const char *topic, const char *data);
    static void _subscribe(esp_mqtt_client_handle_t client, const char *topic);
    static void _unsubscribe(esp_mqtt_client_handle_t client, const char *topic);
    static void _mqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
    static void _logErrorIfNonzero(const char *message, int error_code);
};
#include "mqtt_handler.hpp"

const char* MqttHandler::TAG = "MqttHandler";

void MqttHandler::initializeMqttClient() {
    esp_mqtt_client_config_t mqtt_cfg = {
            .broker = {
                    .address = {
                            .uri = CONFIG_BROKER_URI
                    }
            }
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID), MqttHandler::_mqttEventHandler, nullptr);
    esp_mqtt_client_start(client);
}

void MqttHandler::_publish(esp_mqtt_client_handle_t client, const char *topic, const char *data)
{
    int msg_id = esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
    ESP_LOGI(TAG, "--------------------------------------");
    ESP_LOGI(TAG, "Published message successfully.");
    ESP_LOGI(TAG, "Topic: %s", topic);
    ESP_LOGI(TAG, "Data: %s", data);
    ESP_LOGI(TAG, "Message ID: %d", msg_id);
    ESP_LOGI(TAG, "--------------------------------------");
}

void MqttHandler::_subscribe(esp_mqtt_client_handle_t client, const char *topic)
{
    int msg_id = esp_mqtt_client_subscribe(client, topic, 0);
    ESP_LOGI(TAG, "--------------------------------------");
    ESP_LOGI(TAG, "Subscribed to topic: %s", topic);
    ESP_LOGI(TAG, "Message ID: %d", msg_id);
    ESP_LOGI(TAG, "--------------------------------------");
}

void MqttHandler::_unsubscribe(esp_mqtt_client_handle_t client, const char *topic) {
    int msg_id = esp_mqtt_client_unsubscribe(client, topic);
    ESP_LOGI(TAG, "Unsubscribed from topic: %s\n ID: %d\n", topic, msg_id);
}

void MqttHandler::_mqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    auto event = static_cast<esp_mqtt_event_handle_t>(event_data);
    esp_mqtt_client_handle_t client = event->client;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            MqttHandler::_publish(client, "topic/test_send", "Connected");
            MqttHandler::_subscribe(client, "topic/test_receive");
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "Event: MQTT_EVENT_SUBSCRIBED. Message ID: %d", event->msg_id);
            MqttHandler::_publish(client, "topic/test_send", "Subscribed");
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "Event: MQTT_EVENT_UNSUBSCRIBED. Message ID: %d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Event: MQTT_EVENT_PUBLISHED. Message ID: %d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Event: MQTT_EVENT_DATA");
            ESP_LOGI(TAG, "Topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "Data:\n%.*s", event->data_len, event->data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "Event: MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                MqttHandler::_logErrorIfNonzero("Error reported by esp-tls", event->error_handle->esp_tls_last_esp_err);
                MqttHandler::_logErrorIfNonzero("Error reported by TLS stack", event->error_handle->esp_tls_stack_err);
                MqttHandler::_logErrorIfNonzero("Error captured as transport socket's errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string: (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;

        default:
            ESP_LOGI(TAG, "Event: Other, with ID %d", event->event_id);
            break;
    }
}

void MqttHandler::_logErrorIfNonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Error (0x%x) encountered: %s", error_code, message);
    }
}

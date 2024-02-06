#include <random>
#include "mqtt_handler.hpp"

std::string get_uuid() {
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = {false, false, false, false, true, false, true, false,
                         true, false, true, false, false, false, false, false};

    std::string res;
    for (bool i : dash) {
        if (i) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

const char* MqttHandler::TAG = "MqttHandler";
std::string MqttHandler::UUID = get_uuid();

std::pair<float, float> MqttHandler::speakerHumidityLevels = std::make_pair(20.0f, 80.0f);
std::pair<float, float> MqttHandler::speakerTemperatureLevels = std::make_pair(18.0f, 28.0f);
std::pair<float, float> MqttHandler::diodeHumidityLevels = std::make_pair(20.0f, 80.0f);
std::pair<float, float> MqttHandler::diodeTemperatureLevels = std::make_pair(18.0f, 28.0f);

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
    ESP_LOGI(TAG, "");
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
    ESP_LOGI(TAG, "");
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
            ESP_LOGI(TAG, "Event: Connected");
            MqttHandler::_publish(client, "App/ConnectedUnits", MqttHandler::UUID.c_str());

            MqttHandler::_publish(client, (MqttHandler::UUID + "/Temperature").c_str(), "24.5");
            MqttHandler::_publish(client, (MqttHandler::UUID + "/Humidity").c_str(), "50.0");

            // Subscribe to channels related to speaker playing thresholds
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Speaker/Temperature/MinLimit").c_str());
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Speaker/Temperature/MaxLimit").c_str());
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Speaker/Humidity/MinLimit").c_str());
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Speaker/Humidity/MinLimit").c_str());

            // Subscribe to channels related to diode blinking thresholds
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Diode/Temperature/MinLimit").c_str());
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Diode/Temperature/MaxLimit").c_str());
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Diode/Humidity/MinLimit").c_str());
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Diode/Humidity/MaxLimit").c_str());

            // Additional topic
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Settings/Interval").c_str());
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Settings/PlantData").c_str());
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Plant/Name").c_str());
            MqttHandler::_subscribe(client, (MqttHandler::UUID + "/Plant/Location").c_str());

            /*
            for (int i = 0; i < 1000; i++) {
                vTaskDelay(3000 / portTICK_PERIOD_MS);

                // Get value from normal distribution
                std::random_device rd;
                std::mt19937 rng(rd());
                std::normal_distribution<float> temperatureDistribution(24.0f, 2.0f);
                float temperature = temperatureDistribution(rng);

                std::normal_distribution<float> humidityDistribution(50.0f, 5.0f);
                float humidity = humidityDistribution(rng);

                MqttHandler::_publish(client, (MqttHandler::UUID + "/Temperature").c_str(), std::to_string(temperature).c_str());
                MqttHandler::_publish(client, (MqttHandler::UUID + "/Humidity").c_str(), std::to_string(humidity).c_str());
            }
            */
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Event: Disconnected");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            break;

        case MQTT_EVENT_PUBLISHED:
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Event: Data. Message ID: %d", event->msg_id);
            ESP_LOGI(TAG, "Topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "Data: %.*s", event->data_len, event->data);

            // Check if the message is related to speaker playing thresholds
            if (strncmp(event->topic, (MqttHandler::UUID + "/Speaker/Temperature/MinLimit").c_str(), event->topic_len) == 0) {
                ESP_LOGI(TAG, "Setting minimal temperature limit for speaker");
                MqttHandler::speakerTemperatureLevels.first = std::stof(std::string(event->data, event->data_len));
            }
            else if (strncmp(event->topic, (MqttHandler::UUID + "/Speaker/Temperature/MaxLimit").c_str(), event->topic_len) == 0) {
                ESP_LOGI(TAG, "Setting maximal temperature limit for speaker");
                MqttHandler::speakerTemperatureLevels.second = std::stof(std::string(event->data, event->data_len));
            }
            else if (strncmp(event->topic, (MqttHandler::UUID + "/Speaker/Humidity/MinLimit").c_str(), event->topic_len) == 0) {
                ESP_LOGI(TAG, "Setting minimal humidity limit for speaker");
                MqttHandler::speakerHumidityLevels.first = std::stof(std::string(event->data, event->data_len));
            }
            else if (strncmp(event->topic, (MqttHandler::UUID + "/Speaker/Humidity/MaxLimit").c_str(), event->topic_len) == 0) {
                ESP_LOGI(TAG, "Setting maximal humidity limit for speaker");
                MqttHandler::speakerHumidityLevels.second = std::stof(std::string(event->data, event->data_len));
            }

            // Round to first place on print
            ESP_LOGI(TAG, "------------------------------------------------");
            ESP_LOGI(TAG, "Current speaker temperature limits: %.1f - %.1f", MqttHandler::speakerTemperatureLevels.first, MqttHandler::speakerTemperatureLevels.second);
            ESP_LOGI(TAG, "Current speaker humidity limits:    %.1f - %.1f", MqttHandler::speakerHumidityLevels.first, MqttHandler::speakerHumidityLevels.second);

            // Check if the message is related to diode blinking thresholds
            if (strncmp(event->topic, (MqttHandler::UUID + "/Diode/Temperature/MinLimit").c_str(), event->topic_len) == 0) {
                ESP_LOGI(TAG, "Setting minimal temperature limit for diode");
                MqttHandler::diodeTemperatureLevels.first = std::stof(std::string(event->data, event->data_len));
            }
            else if (strncmp(event->topic, (MqttHandler::UUID + "/Diode/Temperature/MaxLimit").c_str(), event->topic_len) == 0) {
                ESP_LOGI(TAG, "Setting maximal temperature limit for diode");
                MqttHandler::diodeTemperatureLevels.second = std::stof(std::string(event->data, event->data_len));
            }
            else if (strncmp(event->topic, (MqttHandler::UUID + "/Diode/Humidity/MinLimit").c_str(), event->topic_len) == 0) {
                ESP_LOGI(TAG, "Setting minimal humidity limit for diode");
                MqttHandler::diodeHumidityLevels.first = std::stof(std::string(event->data, event->data_len));
            }
            else if (strncmp(event->topic, (MqttHandler::UUID + "/Diode/Humidity/MaxLimit").c_str(), event->topic_len) == 0) {
                ESP_LOGI(TAG, "Setting maximal humidity limit for diode");
                MqttHandler::diodeHumidityLevels.second = std::stof(std::string(event->data, event->data_len));
            }

            ESP_LOGI(TAG, "Current diode temperature limits:   %.1f - %.1f", MqttHandler::diodeTemperatureLevels.first, MqttHandler::diodeTemperatureLevels.second);
            ESP_LOGI(TAG, "Current diode humidity limits:      %.1f - %.1f", MqttHandler::diodeHumidityLevels.first, MqttHandler::diodeHumidityLevels.second);
            ESP_LOGI(TAG, "------------------------------------------------");
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "Event: Error");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                MqttHandler::_logErrorIfNonzero("Error reported by esp-tls", event->error_handle->esp_tls_last_esp_err);
                MqttHandler::_logErrorIfNonzero("Error reported by TLS stack", event->error_handle->esp_tls_stack_err);
                MqttHandler::_logErrorIfNonzero("Error captured as transport socket's errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string: (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;

        default:
            ESP_LOGI(TAG, "Other Event, with ID %d", event->event_id);
            break;
    }
}

void MqttHandler::_logErrorIfNonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Error (0x%x) encountered: %s", error_code, message);
    }
}

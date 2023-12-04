#include <cstring>

#include "http_handler.hpp"

const char* HttpHandler::TAG = "HttpHandler";

void HttpHandler::httpHandlerTask(void *pvParameters) {
    ESP_LOGI(TAG, "Starting HTTP handler task");

    const addrinfo addrHints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
    };
    addrinfo *addressResult;
    in_addr address {};
    int socketFileDescriptor, bytesRead;
    char *buffer = new char[BUFFER_SIZE];

    for(;;) {
        ESP_LOGI(TAG, "HTTP handler task running");

        // Handle Wi-Fi connection ---------------------------------------------
        if (!WifiHandler::connected) {
            ESP_LOGI(TAG, "Waiting for WiFi connection");
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
            continue;
        }

        // Resolve HTTP server address using DNS --------------------------------
        auto addressResultCode = getaddrinfo(WEB_SERVER, WEB_PORT,
                                             &addrHints,&addressResult);
        if (addressResultCode != 0 || addressResult == nullptr) {
            ESP_LOGE(TAG, "Unable to get address info. Return code: %d", addressResultCode);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        address = ((sockaddr_in *) addressResult->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "Address found: %s", inet_ntoa(address));

        // Create socket -------------------------------------------------------
        socketFileDescriptor = socket(addressResult->ai_family, addressResult->ai_socktype, 0);

        if (socketFileDescriptor < 0) {
            ESP_LOGE(TAG, "Unable to create socket. Error code: %d", errno);
            freeaddrinfo(addressResult);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        ESP_LOGI(TAG, "Socket created successfully");

        // Connect to HTTP server ----------------------------------------------
        auto connectionResultCode = connect(socketFileDescriptor, addressResult->ai_addr,
                                            addressResult->ai_addrlen);
        if (connectionResultCode != 0) {
            ESP_LOGE(TAG, "Unable to connect to server. Error code: %d", errno);
            close(socketFileDescriptor);
            freeaddrinfo(addressResult);
            std::this_thread::sleep_for(std::chrono::milliseconds(4000));
            continue;
        }

        freeaddrinfo(addressResult);
        ESP_LOGI(TAG, "Connected to HTTP server");

        // Send HTTP request ---------------------------------------------------
        HttpRequest requestObj = {
                .url = WEB_PATH,
                .version = "1.1",
        };

        add_header(&requestObj, "Host", WEB_SERVER ":" WEB_PORT);
        add_header(&requestObj, "User-Agent", "esp-idf/1.0 esp32");
        const char* request = create_get_request(&requestObj);

        auto requestResultCode = write(socketFileDescriptor, request, strlen(request));

        if (requestResultCode < 0) {
            ESP_LOGE(TAG, "Unable to send request. Error code: %d", errno);
            close(socketFileDescriptor);
            std::this_thread::sleep_for(std::chrono::milliseconds(4000));
            continue;
        }

        ESP_LOGI(TAG, "Request sent successfully");

        // Set socket timeout --------------------------------------------------
        timeval receivingTimeout = { 5, 0 };
        auto receivingTimeoutResultCode = setsockopt(socketFileDescriptor, SOL_SOCKET, SO_RCVTIMEO,
                                                     &receivingTimeout, sizeof(receivingTimeout));
        if (receivingTimeoutResultCode < 0) {
            ESP_LOGE(TAG, "Unable to set socket receiving timeout. Error code: %d", errno);
            close(socketFileDescriptor);
            freeaddrinfo(addressResult);
            std::this_thread::sleep_for(std::chrono::milliseconds(4000));
            continue;
        }

        ESP_LOGI(TAG, "Socket receiving timeout set successfully");

        // Read HTTP response --------------------------------------------------
        ESP_LOGI(TAG, "Reading HTTP response");

        do {
            memset(buffer, 0, BUFFER_SIZE);
            bytesRead = read(socketFileDescriptor, buffer, BUFFER_SIZE - 1);
            for (int i = 0; i < bytesRead; i++) {
                putchar(buffer[i]);
            }
        } while (bytesRead > 0);

        ESP_LOGI(TAG, "HTTP response read successfully");

        // Close connection ----------------------------------------------------
        ESP_LOGI(TAG, "Closing connection");

        close(socketFileDescriptor);

        ESP_LOGI(TAG, "Connection closed successfully");

        for (int countdown = 20; countdown >= 0; countdown--) {
            if (countdown % 5 == 0 && countdown > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}

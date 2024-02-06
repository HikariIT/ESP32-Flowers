#include <cstring>

#include "http_handler.hpp"

const char* HttpHandler::TAG = "HttpHandler";

void HttpHandler::httpHandlerTask(void *pvParameters) {

}

HttpResponse* HttpHandler::makeGetRequest(const char *message, const char *endpoint, const char *path, const char *port) {
    return makeRequest(message, endpoint, path, port, "", HttpMethod::GET);
}

HttpResponse* HttpHandler::makePostRequest(const char *message, const char *endpoint, const char *path, const char *port, const char *body) {
    return makeRequest(message, endpoint, path, port, body, HttpMethod::POST);
}

HttpResponse* HttpHandler::makeDeleteRequest(const char *message, const char *endpoint, const char *path, const char *port, const char *body) {
    return makeRequest(message, endpoint, path, port, body, HttpMethod::DELETE);
}

HttpResponse* HttpHandler::makeRequest(const char *message, const char *endpoint, const char *path, const char *port, const char *body, HttpMethod method) {
    // Initialize variables ---------------------------------------------------
    const addrinfo addrHints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
    };
    addrinfo *addressResult;
    in_addr address {};
    int socketFileDescriptor, bytesRead;
    char *buffer = new char[BUFFER_SIZE];

    for (int i = 0; i < MAX_RETRY_COUNT; i++) {
        // Handle Wi-Fi connection --------------------------------------------
        if (!WifiHandler::connected) {
            ESP_LOGI(TAG, "Waiting for WiFi connection");
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY));
            continue;
        }

        // Resolve HTTP server address using DNS ------------------------------
        auto addressResultCode = getaddrinfo(endpoint, port, &addrHints, &addressResult);
        if (addressResultCode != 0 || addressResult == nullptr) {
            ESP_LOGE(TAG, "Unable to get address info. Return code: %d", addressResultCode);
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY));
            continue;
        }

        address = ((sockaddr_in *) addressResult->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "Address found: %s", inet_ntoa(address));

        // Create socket -------------------------------------------------------
        socketFileDescriptor = socket(addressResult->ai_family, addressResult->ai_socktype, 0);

        if (socketFileDescriptor < 0) {
            ESP_LOGE(TAG, "Unable to create socket. Error code: %d", errno);
            freeaddrinfo(addressResult);
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY));
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
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY));
            continue;
        }

        freeaddrinfo(addressResult);
        ESP_LOGI(TAG, "Connected to HTTP server");

        // Send HTTP request ---------------------------------------------------
        HttpRequest requestObj = {
                .url = path,
                .version = "1.1",
        };

        char *pathPort = new char[strlen(path) + strlen(port) + 2];
        strcpy(pathPort, path);
        strcat(pathPort, ":");
        strcat(pathPort, port);

        add_header(&requestObj, "Host", pathPort);
        add_header(&requestObj, "User-Agent", "esp-idf/1.0 esp32");
        const char* request = nullptr;

        switch (method) {
            case HttpMethod::GET:
                request = create_get_request(&requestObj);
                break;
            case HttpMethod::POST:
                add_header(&requestObj, "Content-Type", "application/json");
                add_header(&requestObj, "Content-Length", std::to_string(strlen(body)).c_str());
                request = create_post_request(&requestObj, body);
                break;
            case HttpMethod::DELETE:
                request = " "; // Placeholder
                break;
        }

        auto requestResultCode = write(socketFileDescriptor, request, strlen(request));

        if (requestResultCode < 0) {
            ESP_LOGE(TAG, "Unable to send request. Error code: %d", errno);
            close(socketFileDescriptor);
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY));
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
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY));
            continue;
        }

        ESP_LOGI(TAG, "Socket receiving timeout set successfully");

        // Read HTTP response --------------------------------------------------
        ESP_LOGI(TAG, "Reading HTTP response");

        // Store full response
        std::string fullResponse;

        do {
            memset(buffer, 0, BUFFER_SIZE);
            bytesRead = read(socketFileDescriptor, buffer, BUFFER_SIZE - 1);
            for (int bufferPos = 0; bufferPos < bytesRead; bufferPos++) {
                putchar(buffer[bufferPos]);
                fullResponse += buffer[bufferPos];
            }
        } while (bytesRead > 0);

        auto httpResponse = new HttpResponse(fullResponse);

        ESP_LOGI(TAG, "HTTP response read successfully");
        ESP_LOGI(TAG, "Code        %d", httpResponse->status_code);
        ESP_LOGI(TAG, "Version     %s", httpResponse->version);
        ESP_LOGI(TAG, "Headers     \n%s", httpResponse->headers);
        ESP_LOGI(TAG, "Body        \n%s", httpResponse->body);

        // Close connection ----------------------------------------------------
        ESP_LOGI(TAG, "Closing connection");

        close(socketFileDescriptor);

        ESP_LOGI(TAG, "Connection closed successfully");

        return httpResponse;
    }

    return nullptr;
}
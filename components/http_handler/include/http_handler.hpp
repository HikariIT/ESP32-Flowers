#pragma once

#include "wifi_handler.hpp"
#include "http_request.hpp"
#include "http_response.hpp"

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <esp_log.h>
#include <thread>

#define WEB_SERVER      "worldtimeapi.org"
#define WEB_PATH        "/"
#define BUFFER_SIZE     1024
#define RETRY_DELAY     1000
#define MAX_RETRY_COUNT 10

enum class HttpMethod {
    GET,
    POST,
    DELETE,
};

class HttpHandler {
    static const char* TAG;
public:
    static HttpResponse* makeGetRequest(const char *message, const char *endpoint, const char *path, const char *port);
    static HttpResponse* makePostRequest(const char *message, const char *endpoint, const char *path, const char *port, const char *body);
    static HttpResponse* makeDeleteRequest(const char *message, const char *endpoint, const char *path, const char *port, const char *body);
    static HttpResponse* makeRequest(const char *message, const char *endpoint, const char *path, const char *port, const char *body, HttpMethod method);
    static void httpHandlerTask(void *pvParameters);
};


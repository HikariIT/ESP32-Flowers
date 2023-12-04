#pragma once

#include "wifi_handler.hpp"
#include "http_request.hpp"

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <esp_log.h>
#include <thread>

#define WEB_SERVER      "worldtimeapi.org"
#define WEB_PORT        "80"
#define WEB_PATH        "/"
#define BUFFER_SIZE     1024

class HttpHandler {
    static const char* TAG;
public:
    static void httpHandlerTask(void *pvParameters);
};




#include <esp_log.h>
#include "http_response.hpp"

HttpResponse::HttpResponse(const std::string& buffer) {
    version = nullptr;
    status_code = 0;
    header_count = 0;
    headers = nullptr;
    body = nullptr;
    size_t response_length = buffer.length();

    ESP_LOGI("HttpResponse", "Response length: %d", response_length);

    size_t bufferPos = 0;
    size_t lineStart = 0;
    size_t lineEnd = 0;
    size_t headerStart = 0;
    size_t headerEnd = 0;
    size_t bodyStart = 0;
    size_t bodyEnd = 0;

    bool parsingHeaders = false;

    while (bufferPos < response_length) {
        if (buffer[bufferPos] == '\n') {
            lineEnd = bufferPos;
            // Handle first line
            if (lineStart == 0) {
                // Version is first word
                size_t versionEnd = buffer.find(' ', lineStart);
                version = (char*) malloc(versionEnd - lineStart + 1);
                memcpy(version, buffer.c_str() + lineStart, versionEnd - lineStart);
                version[versionEnd - lineStart] = '\0';

                // Status code is second word and is 3 characters long
                size_t statusCodeStart = versionEnd + 1;
                status_code = std::stoi(buffer.substr(statusCodeStart, 3));
                parsingHeaders = true;
                headerStart = bufferPos + 1;
            } else if (parsingHeaders) {
                if (buffer[lineStart] == '\r') {
                    // End of headers
                    headerEnd = lineStart - 1;
                    bodyStart = bufferPos + 1;
                    parsingHeaders = false;
                    break;
                } else {
                    // Header
                    header_count++;
                }
            } else {
                // Rest is body
                bodyStart = bufferPos + 1;
                break;
            }
            lineStart = bufferPos + 1;
        }
        bufferPos++;
    }

    headers = (char*) malloc(headerEnd - headerStart + 1);
    memcpy(headers, buffer.c_str() + headerStart, headerEnd - headerStart);
    headers[headerEnd - headerStart] = '\0';

    body = (char*) malloc(response_length - bodyStart + 1);
    memcpy(body, buffer.c_str() + bodyStart, response_length - bodyStart);
    body[response_length - bodyStart] = '\0';
}

HttpResponse::~HttpResponse() {
    free(headers);
    free(body);
}


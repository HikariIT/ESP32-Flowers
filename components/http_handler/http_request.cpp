#include "http_request.hpp"

const char* create_get_request(HttpRequest *request) {
    size_t length = 0;
    length += strlen("GET ") + strlen(request->url) + strlen(" HTTP/") + strlen(request->version) + strlen("\r\n");

    for (size_t i = 0; i < request->header_count; i++)
        length += strlen(request->headers[i]);

    length += strlen("\r\n");

    // ---------------------------------------------

    char *output = (char*) malloc(length + 1);
    if (output == nullptr) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    snprintf(output, length + 1, "GET %s HTTP/%s\r\n", request->url, request->version);

    for (size_t i = 0; i < request->header_count; i++)
        strncat(output, request->headers[i], length + 1);

    strncat(output, "\r\n", length + 1);
    return output;
}

void add_header(HttpRequest *request, const char *name, const char *value) {
    if (request->header_count < MAX_HEADERS) {
        request->headers[request->header_count] = (char*) malloc(strlen(name) + strlen(value) + 5); // 5 for ": \r\n\0"

        if (request->headers[request->header_count] == nullptr) {
            fprintf(stderr, "Memory allocation error\n");
            exit(1);
        }

        snprintf(request->headers[request->header_count], strlen(name) + strlen(value) + 5, "%s: %s\r\n", name, value);
        request->header_count++;
    } else {
        fprintf(stderr, "Maximum number of headers reached\n");
    }
}


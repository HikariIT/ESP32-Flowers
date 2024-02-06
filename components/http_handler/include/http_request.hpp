#pragma once

#define MAX_HEADERS         10
#define MAX_CONTENT_LENGTH  1000

#include <cstdio>
#include <cstdlib>
#include <cstring>

struct HttpRequest {
    const char *url;
    const char *version;
    size_t header_count;
    char *headers[MAX_HEADERS];
};

const char* create_get_request(struct HttpRequest *request);
const char* create_post_request(struct HttpRequest *request, const char *body);

void add_header(struct HttpRequest *request, const char *name, const char *value);


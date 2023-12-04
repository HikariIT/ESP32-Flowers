#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#define MAX_HEADERS         10

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

void add_header(struct HttpRequest *request, const char *name, const char *value);

#endif

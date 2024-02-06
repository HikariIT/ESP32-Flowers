#pragma once

#include <cstdio>
#include <cstring>
#include <string>

class HttpResponse {
public:
    char *version;
    int status_code;
    size_t header_count;
    char *headers;
    char *body;

    explicit HttpResponse(const std::string& buffer);
    ~HttpResponse();
};
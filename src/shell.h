#ifndef ___SHELL_H___
#define ___SHELL_H___

#include "file_op.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef REDIS_HOST_FILE
#define REDIS_HOST_FILE "redis.host"
#endif // !REDIS_HOST_FILE

const char* host_get(const char* address) {
    static char _buf[16];
    char curl[1024];
    snprintf(curl, sizeof(curl), "curl --noproxy \"*\" %s/get > "
        REDIS_HOST_FILE " 2> /dev/null", address);
    if (system(curl)) {
        fprintf(stderr, "[Error] curl run failed\n");
        return "";
    }
    size_t len = 0;
    char* redis_host = get_file(REDIS_HOST_FILE, &len);
    while (len > 0 && isspace(redis_host[len - 1])) {
        --len;
    }
    if (len >= sizeof(_buf)) {
        fprintf(stderr, "[Error] curl get failed\n");
        len = 0;
    } else {
        memcpy(_buf, redis_host, len);
    }
    free_file(redis_host);
    _buf[len] = '\0';
    return _buf;
}

#endif // ! ___SHELL_H___

#ifndef ___REDIS_OP_H___
#define ___REDIS_OP_H___

#include "zlib_op.h"

#include <hiredis.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef REDIS_QUEUE_KEY
#define REDIS_QUEUE_KEY "task"
#endif // !REDIS_QUEUE_KEY
#ifndef REDIS_RETRY_TIMES
#define REDIS_RETRY_TIMES 3
#endif // !REDIS_RETRY_TIMES
#ifndef REDIS_RETRY_TIMEOUT
#define REDIS_RETRY_TIMEOUT "1"
#endif // !REDIS_RETRY_TIMEOUT

static redisContext* _redis_conn;

int redis_init(int argc, char **argv) {
    _redis_conn = NULL;
    unsigned int isunix = 0;
    const char *hostname = (argc > 1) ? argv[1] : "10.186.5.116";

    if (argc > 2) {
        if (*argv[2] == 'u' || *argv[2] == 'U') {
            isunix = 1;
            /* in this case, host is the path to the unix socket */
            fprintf(stdout, "[Warning] Will connect to unix socket @%s\n", hostname);
        }
    }

    int port = (argc > 2) ? atoi(argv[2]) : 6379;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    _redis_conn = isunix? redisConnectUnixWithTimeout(hostname, timeout)
                        : redisConnectWithTimeout(hostname, port, timeout);
    if (_redis_conn == NULL) {
        fprintf(stderr, "[Error] Connection error: can't allocate redis context\n");
        return 0;
    }
    if (_redis_conn->err) {
        fprintf(stderr, "[Error] Connection error: %s\n", _redis_conn->errstr);
        redisFree(_redis_conn);
        _redis_conn = NULL;
        return 0;
    }
    return 1;
}

int redis_destroy() {
    redisFree(_redis_conn);
    return 0;
}

int redis_push(const char* bytes, unsigned long len) {
#ifdef DEBUG_CHECK
    if (!_redis_conn) {
        return 0;
    }
#endif // !DEBUG_CHECK
    bytes = compress_bytes(bytes, &len);
    for (unsigned test = REDIS_RETRY_TIMES; test--; ) {
        redisReply* reply = redisCommand(_redis_conn,
            "LPUSH " REDIS_QUEUE_KEY " %b", bytes, len);
        if (reply->type == REDIS_REPLY_ERROR) {
            fprintf(stderr, "[Error] LPUSH: %s\n", reply->str);
            freeReplyObject(reply);
        } else {
            freeReplyObject(reply);
            break;
        }
    }
    return 0;
}

const char* redis_pop() {
#ifdef DEBUG_CHECK
    if (!_redis_conn) {
        return NULL;
    }
#endif // !DEBUG_CHECK
    for (unsigned test = REDIS_RETRY_TIMES; test--; ) {
        redisReply* reply = redisCommand(_redis_conn,
            "BRPOP " REDIS_QUEUE_KEY " " REDIS_RETRY_TIMEOUT);
        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            unsigned long len = reply->element[1]->len;
            const char* str = uncompress_bytes(reply->element[1]->str, &len);
#ifdef DEBUG_CHECK
            if (len > STRING_MAX_LEN) {
                fprintf(stderr, "[Error] BRPOP: len(data): %lu\n", len);
            }
#endif // !DEBUG_CHECK
            freeReplyObject(reply);
            return str;
        } else {
            fprintf(stderr, "[Error] BRPOP: %s\n", reply->str);
            freeReplyObject(reply);
        }
    }
    return NULL;
}


#endif // ! ___REDIS_OP_H___

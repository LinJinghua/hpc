#ifndef ___REDIS_OP_H___
#define ___REDIS_OP_H___

#include <hiredis.h>
#include <stdlib.h>
#include <stdio.h>

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

int redis_push() {
#ifdef DEBUG_CHECK
    if (!_redis_conn) {
        return 0;
    }
#endif // !DEBUG_CHECK
    for (int test = 3; test--; ) {
        redisReply* reply = redisCommand(_redis_conn, "LPUSH tasks %b", "a", 1);
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


#endif // ! ___REDIS_OP_H___

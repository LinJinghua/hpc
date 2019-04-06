#ifndef ___REDIS_OP_H___
#define ___REDIS_OP_H___

#ifndef DEBUG_CHECK
#define NDEBUG
#endif // !DEBUG_CHECK

#include "zinc.h"
#include "zlib_op.h"

#include <hiredis.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#ifndef REDIS_RETRY_TIMES
#define REDIS_RETRY_TIMES 3
#endif // !REDIS_RETRY_TIMES
#ifndef REDIS_RETRY_TIMEOUT
#define REDIS_RETRY_TIMEOUT "1"
#endif // !REDIS_RETRY_TIMEOUT
#ifndef REDIS_PORT
#define REDIS_PORT 6379
#endif // !REDIS_PORT

static redisContext* _redis_conn;

int redis_check() {
    if (_redis_conn == NULL) {
        fprintf(stderr, "[Error] Redis connection error: can't allocate redis context\n");
        return 0;
    }
    if (_redis_conn->err) {
        fprintf(stderr, "[Error] Redis connection error: %s\n", _redis_conn->errstr);
        redisFree(_redis_conn);
        _redis_conn = NULL;
        return 0;
    }
    return 1;
}

int redis_connect(const char* hostname, int port) {
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    _redis_conn = redisConnectWithTimeout(hostname, port, timeout);
    return redis_check();
}

int redis_init(int argc, char **argv) {
    _redis_conn = NULL;
    unsigned int isunix = 0;
    const char* hostname = (argc > 1) ? argv[1] : "127.0.0.1";

    if (argc > 2) {
        if (*argv[2] == 'u' || *argv[2] == 'U') {
            isunix = 1;
            /* in this case, host is the path to the unix socket */
            fprintf(stdout, "[Warning] Will connect to unix socket @%s\n", hostname);
        }
    }

    int port = (argc > 2) ? atoi(argv[2]) : REDIS_PORT;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    _redis_conn = isunix? redisConnectUnixWithTimeout(hostname, timeout)
                        : redisConnectWithTimeout(hostname, port, timeout);
    return redis_check();
}

int redis_destroy() {
    redisFree(_redis_conn);
    return 0;
}

const char* redis_get_name(const char* key) {
    assert(_redis_conn);
    assert(key);
    redisReply* reply = redisCommand(_redis_conn, "GET %s", key);
    if (!reply) {
        fprintf(stderr, "[Error] Run redis GET command failed\n");
        return 0;
    }
    if (reply->type == REDIS_REPLY_STRING) {
        unsigned long len = reply->len;
#ifdef DEBUG_CHECK
        if (len > STRING_MAX_LEN) {
            fprintf(stderr, "[Error] Redis GET %s: len(data): %lu\n", key, len);
        }
#endif // !DEBUG_CHECK
        const char* str = uncompress_bytes(reply->str, &len);
        freeReplyObject(reply);
        return str;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "[Error] Redis GET %s: %s\n", key, reply->str);
    }
    freeReplyObject(reply);
    return NULL;
}

int redis_set(const char* key, const char* value, unsigned long len) {
    assert(_redis_conn);
    assert(key);
    assert(value);
    value = compress_bytes(value, &len);
    redisReply* reply = redisCommand(_redis_conn,
        "SET %s %b", key, value, len);
    if (!reply) {
        fprintf(stderr, "[Error] Run redis SET command failed\n");
        return 0;
    }
    if (reply->type != REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return 1;
    }
    fprintf(stderr, "[Error] Redis SET %s: %s\n", key, reply->str);
    freeReplyObject(reply);
    return 0;
}


double redis_zscore(const char* key) {
    assert(_redis_conn);
    assert(key);
    redisReply* reply = redisCommand(_redis_conn,
        "ZSCORE %s %s", entry_id_get(), key);
    if (!reply) {
        fprintf(stderr, "[Error] Run redis GET command failed\n");
        return 0;
    }
    if (reply->type == REDIS_REPLY_STRING) {
        double score = strtod(reply->str, NULL);
        freeReplyObject(reply);
        return score;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "[Error] Redis ZSCORE _%s %s: %s\n",
                            entry_id_get(), key, reply->str);
    }
    freeReplyObject(reply);
    return NAN;
}

int redis_record(const char* key, const double score, const char* value, unsigned long len) {
    assert(_redis_conn);
    assert(key);
    assert(value);
    const size_t MAX_DOUBLE_LEN = 64;
    char buf[MAX_DOUBLE_LEN];
    snprintf(buf, sizeof(buf), "%a", score);
    redisReply* reply = redisCommand(_redis_conn,
        "ZADD _%s %s %s", entry_id_get(), buf, key);
    if (!reply) {
        fprintf(stderr, "[Error] Run redis ZADD command failed\n");
        return 0;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "[Error] Redis ZADD _%s %s: %s\n",
                            entry_id_get(), key, reply->str);
        freeReplyObject(reply);
    } else {
        freeReplyObject(reply);
        return redis_set(key, value, len);
    }
    return 0;
}

int redis_push(const char* bytes, unsigned long len) {
    assert(_redis_conn);
    if (!bytes) {
        return 0;
    }
    bytes = compress_bytes(bytes, &len);
    for (unsigned test = REDIS_RETRY_TIMES; test--; ) {
        redisReply* reply = redisCommand(_redis_conn,
            "LPUSH %s %b", entry_id_get(), bytes, len);
        if (!reply) {
            fprintf(stderr, "[Error] Run redis LPUSH command failed\n");
            return 0;
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            fprintf(stderr, "[Error] Redis LPUSH %s: %s\n",
                                entry_id_get(), reply->str);
            freeReplyObject(reply);
        } else {
            freeReplyObject(reply);
            return 1;
        }
    }
    return 0;
}

const char* redis_pop() {
    assert(_redis_conn);
    for (unsigned test = REDIS_RETRY_TIMES; test--; ) {
        redisReply* reply = redisCommand(_redis_conn,
            "BRPOP %s " REDIS_RETRY_TIMEOUT, entry_id_get());
        if (!reply) {
            fprintf(stderr, "[Error] Run redis BRPOP command failed\n");
            return NULL;
        }
        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            unsigned long len = reply->element[1]->len;
#ifdef DEBUG_CHECK
            if (len > STRING_MAX_LEN) {
                fprintf(stderr, "[Error] Redis BRPOP: len(data): %lu\n", len);
            }
#endif // !DEBUG_CHECK
            const char* str = uncompress_bytes(reply->element[1]->str, &len);
            freeReplyObject(reply);
            return str;
        } else {
            fprintf(stderr, "[Error] Redis BRPOP %s: %s\n",
                                entry_id_get(), reply->str);
            freeReplyObject(reply);
        }
    }
    return NULL;
}


#endif // ! ___REDIS_OP_H___

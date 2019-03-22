#include "redis_op.h"
#include "mongo_op.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int init_env(int argc, char **argv) {
    return redis_init(argc, argv) && mongo_init(argc, argv);
}

int destory_env() {
    redis_destroy();
    mongo_destroy();
    return 0;
}

int redis_get() {
    for (const char* str = redis_pop(); str; str = redis_pop()) {
        data_entry entry = data_entry_get(str);
        printf("name: %s data: %s\n", entry.name, entry.data);
    }
    return 1;
}

int mongo_get() {
#ifdef TEST_REDIS_RW
    redis_get();
#endif // !TEST_REDIS_RW
    bson_t* query = bson_new();
    mongoc_cursor_t* cursor = mongoc_collection_find_with_opts(
                        _mongo_res.collection, query, NULL, NULL);
    bson_destroy(query);

    bson_error_t error;
    if (mongoc_cursor_error(cursor, &error)) {
        fprintf(stderr, "[Error] An error occurred: %s\n", error.message);
        return 0;
    }

    const bson_t* doc;
    data_entry entry;
    while (mongoc_cursor_next(cursor, &doc)) {
        data_entry_init(&entry);
        unsigned long len = data_entry_set(&entry, doc);
        redis_push(data_entry_string(&entry, len), len);
#ifdef TEST_REDIS_RW
        const char* str = redis_pop();
        if (!str) {
            fprintf(stderr, "[Error] Redis can not write\n");
        }
        data_entry entry_redis = data_entry_get(str);
        if (data_entry_cmp(&entry, &entry_redis)) {
            fprintf(stderr, "[Error] Redis diff read/write\n");
            printf("name: %s : %s\n", entry.name, entry_redis.name);
            printf("data: %s : %s\n", entry.data, entry_redis.data);
            exit(1);
        }
#endif // !TEST_REDIS_RW
    }

    mongoc_cursor_destroy(cursor);
    return 1;
}

int process() {
    mongo_get();
    // redis_get();
    return 0;
}

int main(int argc, char **argv) {
    if (init_env(argc, argv)) {
        process();
    }
    destory_env();
    return 0;
}

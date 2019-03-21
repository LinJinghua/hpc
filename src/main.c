#include "redis_op.h"
#include "mongo_op.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int init_env(int argc, char **argv) {
    return redis_init(argc, argv) && mongo_init();
}

int destory_env() {
    redis_destroy();
    mongo_destroy();
    return 0;
}

int mongo_get() {
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
    zinc_entry entry;
    while (mongoc_cursor_next(cursor, &doc)) {
        zinc_entry_init(&entry);
        unsigned long len = zinc_entry_set(&entry, doc);
        redis_push(zinc_entry_string(&entry, len), len);
#ifdef TEST_REDIS_RW
        const char* str = redis_pop();
        if (!str) {
            fprintf(stderr, "[Error] Redis can not write\n");
        }
        zinc_entry entry_redis = zinc_entry_get(str);
        if (zinc_entry_cmp(&entry, &entry_redis)) {
            fprintf(stderr, "[Error] Redis diff read/write\n");
        } else {
            printf("[Pass] TEST_REDIS_RW\n");
        }
#endif // !TEST_REDIS_RW
    }

    mongoc_cursor_destroy(cursor);
    return 1;
}

int redis_get() {
    for (const char* str = redis_pop(); str; str = redis_pop()) {
        zinc_entry entry = zinc_entry_get(str);
        printf("index_name: %s num_atoms: %d\n", entry.index_name, atoi(entry.num_atoms));
    }
    return 1;
}

int process() {
    mongo_get();
    redis_get();
    return 0;
}

int main(int argc, char **argv) {
    if (init_env(argc, argv)) {
        process();
    }
    destory_env();
    return 0;
}

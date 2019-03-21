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

const char* entry_to_string() {
    return NULL;
}

int get_entry() {
    bson_t* query = bson_new();
    // BSON_APPEND_INT32(query, "foo", 123);
    // bson_t* opts = BCON_NEW("limit", BCON_INT64(0));
    mongoc_cursor_t* cursor = mongoc_collection_find_with_opts(
                        _mongo_res.collection, query, NULL, NULL);
    // mongoc_cursor_t* cursor = mongoc_collection_find(_mongo_res.collection,
    //                          MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    // bson_destroy(opts);
    bson_destroy(query);

    const bson_t* doc;
    while (mongoc_cursor_next(cursor, &doc)) {
        // char* str = bson_as_canonical_extended_json(doc, NULL);
        // printf("%s\n", str);
        // bson_free(str);
        bson_iter_t iter;
        if (!bson_iter_init(&iter, doc)) {
            continue;
        }
        while (bson_iter_next(&iter)) {
            if (BSON_ITER_HOLDS_INT32(&iter)) {
                printf("Found element key: \"{%s : %d}\"\n",
                    bson_iter_key(&iter), bson_iter_int32(&iter));
            } else if (BSON_ITER_HOLDS_UTF8(&iter)) {
                unsigned int len;
                const char* str = bson_iter_utf8(&iter, &len);
                if (len < 32) {
                    printf("Found element key: \"{%s : %s}\"\n",
                        bson_iter_key(&iter), str);
                } else {
#ifdef TEST_ZLIB
                    char tmp_buf[20000];
                    unsigned long comp_len = len;
                    const char* comp_str = compress_bytes(str, &comp_len);
                    memcpy(tmp_buf, comp_str, comp_len);
                    unsigned long uncomp_len = comp_len;
                    const char* uncomp_str = uncompress_bytes(tmp_buf, &uncomp_len);
                    if (uncomp_len != len || strcmp(uncomp_str, str)) {
                        printf("[Error] compress: %lu %u\n", uncomp_len, len);
                    }
                    printf("Found element key: \"{%s : Rate(%f,%lu/%u)}\"\n",
                        bson_iter_key(&iter), (double)comp_len / len, comp_len, len);
#else
                    printf("Found element key: \"{%s : %s}\"\n",
                        bson_iter_key(&iter), "Too Long...");
#endif // !TEST_ZLIB
                }
            } else if (BSON_ITER_HOLDS_OID(&iter)) {
                char oidstr[25];
                const bson_oid_t* oid = bson_iter_oid(&iter);
                bson_oid_to_string(oid, oidstr);
                printf("Found element key: \"{%s : %s}\"\n",
                    bson_iter_key(&iter), oidstr);
            } else {
                printf("Found element key: \"{%s : Unkonwn}\"\n",
                    bson_iter_key(&iter));
            }
        }
    }

    bson_error_t error;
    if (mongoc_cursor_error(cursor, &error)) {
        fprintf(stderr, "[Error] An error occurred: %s\n", error.message);
    }

    mongoc_cursor_destroy(cursor);
    
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

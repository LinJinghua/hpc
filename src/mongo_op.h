#ifndef ___MONGO_OP_H___
#define ___MONGO_OP_H___

#include "zinc.h"

#include <mongoc.h>
#include <bson.h>
#include <stdlib.h>
#include <string.h>


#define CHECK_MONGO_ERR(condition, ...) do {                \
    if (condition) {                                        \
        fprintf(stderr, ##__VA_ARGS__);                     \
        return 0;                                           \
    }                                                       \
} while (0)


typedef struct mongo_res {
    mongoc_uri_t* uri;
    mongoc_client_t* client;
    mongoc_database_t* database;
    mongoc_collection_t* collection;
} mongo_res;
static mongo_res _mongo_res;

int mongo_res_int() {
    _mongo_res.uri = NULL;
    _mongo_res.client = NULL;
    _mongo_res.database = NULL;
    _mongo_res.collection = NULL;
    return 0;
}
int mongo_res_free() {
    mongoc_collection_destroy(_mongo_res.collection);
    mongoc_database_destroy(_mongo_res.database);
    mongoc_uri_destroy(_mongo_res.uri);
    mongoc_client_destroy(_mongo_res.client);
    return 0;
}


int mongo_init(int argc, char **argv) {
    const char *uri_string = argc > 3? argv[3] : "mongodb://127.0.0.1:27017";
    const char *database_name = argc > 4? argv[4] : "zinc_data";
    const char *collection_name = argc > 5? argv[5] : "zinc_ligand_1w_sort";
    mongo_idxd_set(argc > 6? atoi(argv[6]) : 0);
    entry_id_set(database_name, collection_name);
    if (argc > 7) {
        entry_id_set_field(argv[6], argv[7]);
    } else {
        if (database_name[0] == 'z') {
            entry_id_set_field("index_name", "pdbqt_file");
        } else {
            entry_id_set_field("sdf_name", "sdf_data");
        }
    };

    mongo_res_int();
    mongoc_init();

    bson_error_t error;
    _mongo_res.uri = mongoc_uri_new_with_error(uri_string, &error);
    CHECK_MONGO_ERR(!_mongo_res.uri, "[Error] failed to parse URI: %s\nerror message: %s\n",
        uri_string, error.message);

    _mongo_res.client = mongoc_client_new_from_uri(_mongo_res.uri);
    CHECK_MONGO_ERR(!_mongo_res.client, "[Error] failed to get client\n");

    bson_t reply, *command = BCON_NEW("ping", BCON_INT32(1));
    bool retval = mongoc_client_command_simple(_mongo_res.client, "admin", command, NULL, &reply, &error);
    bson_destroy(command);
    CHECK_MONGO_ERR(!retval, "[Error] failed to run command %s\n", error.message);
    char* str = bson_as_json(&reply, NULL);
    printf("[Mongo] %s\n", str);
    bson_free(str);
    bson_destroy(&reply);

    // _mongo_res.database = mongoc_client_get_database(client, "db_name");
    _mongo_res.collection = mongoc_client_get_collection(
        _mongo_res.client, database_name, collection_name);

    return 1;
}

int mongo_destroy() {
    mongo_res_free();
    mongoc_cleanup();
    return 0;
}

unsigned int data_entry_set(data_entry* entry, const bson_t* doc) {
    bson_iter_t iter;
    if (!bson_iter_init(&iter, doc)) {
        return 0;
    }
    const char* name_field = entry_id_get_name_field();
    const char* data_field = entry_id_get_data_field();
    unsigned int total_len = 1;
    while (bson_iter_next(&iter)) {
        const char* key = bson_iter_key(&iter);
        if (strcmp(key, name_field) == 0) {
            if (BSON_ITER_HOLDS_UTF8(&iter)) {
                unsigned int len;
                entry->name = bson_iter_utf8(&iter, &len);
                total_len += len;
            } else {
                fprintf(stderr, "[Error] Type(%s) error\n", name_field);
            }
        } else if (strcmp(key, data_field) == 0) {
            if (BSON_ITER_HOLDS_UTF8(&iter)) {
                unsigned int len;
                entry->data = bson_iter_utf8(&iter, &len);
                total_len += len;
            } else {
                fprintf(stderr, "[Error] Type(%s) error\n", data_field);
            }
        }
    }
    return total_len;
}


int mongo_get_all_entry() {
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


#endif // ! ___MONGO_OP_H___

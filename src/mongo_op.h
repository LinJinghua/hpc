#ifndef ___MONGO_OP_H___
#define ___MONGO_OP_H___

#include <mongoc.h>
#include <bson.h>

#ifndef DATABASE_NAME
#define DATABASE_NAME "zinc_data"
#endif // ! DATABASE_NAME
#ifndef COLLECTION_NAME
#define COLLECTION_NAME "zinc_ligand_1w_sort"
#endif // ! COLLECTION_NAME


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


int mongo_init() {
    mongo_res_int();
    mongoc_init();

    // const char *uri_string = "mongodb://12.11.70.12:10101";
    const char *uri_string = "mongodb://127.0.0.1:27017";
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
    _mongo_res.collection = mongoc_client_get_collection(_mongo_res.client, DATABASE_NAME, COLLECTION_NAME);

    return 1;
}

int mongo_destroy() {
    mongo_res_free();
    mongoc_cleanup();
    return 0;
}

#endif // ! ___MONGO_OP_H___

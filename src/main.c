#include "redis_op.h"
#include "mongo_op.h"
#include "shell.h"
#include "vina.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int init_producer_mongo(int argc, char **argv) {
    return redis_init(argc, argv) && mongo_init(argc, argv);
}

int init_cluster(int argc, char **argv) {
    const char* schedule_addr = (argc > 1) ? argv[1] : "127.0.0.1:8080";
    const char* redis_queue = (argc > 2) ? argv[2] : "task";
    entry_id_check(redis_queue);
    return vina_init() && make_work_dir()
        && redis_connect(host_get(schedule_addr), REDIS_PORT);
}

int init_producer(int argc, char **argv, data_entry* entry) {
    const char* schedule_addr = (argc > 1) ? argv[1] : "127.0.0.1:8080";
    const char* redis_queue = (argc > 2) ? argv[2] : "task";
    entry_id_check(redis_queue);
    entry->name = (argc > 3) ? argv[3] : "target";
    entry->data = (argc > 4) ? argv[4] : "target.pdbqt";
    return redis_connect(host_get(schedule_addr), REDIS_PORT);
}

int destory_producer() {
    redis_destroy();
    return 0;
}

int destory_cluster() {
    redis_destroy();
    destory_work_dir();
    return 0;
}

int destory_producer_mongo() {
    redis_destroy();
    mongo_destroy();
    return 0;
}

int write_result(const char* name, const char* mol_file) {
    size_t len;
    char* data = file_str(mol_file, &len);
    return data && redis_record(name, data, len);
}

int redis_get() {
    char name[_ID_MAX_LEN];
    for (const char* str = redis_pop(); str; str = redis_pop()) {
        data_entry entry = data_entry_get(str);
        strncpy(name, entry.name, sizeof(name) - 1);
        // printf("name: %s data: %s\n", entry.name, entry.data);
        const char* vina_file = "vina_file";
        const char* pdbqt_file = "pdbqt_file";
        const char* mol_file = "mol_file";
        if (write_file(vina_file, entry.data, strlen(entry.data))
            && vina_run(vina_file, pdbqt_file) == 0
            && obabel_run(pdbqt_file, mol_file) == 0
            && write_result(name, mol_file)) {
            fprintf(stdout, "[Info] Run task %s succ\n", name);
        } else {
            fprintf(stderr, "[Error] Run task %s failed\n", name);
        }
    }
    return 0;
}

int file_get(data_entry* entry) {
    size_t len;
    entry->data = file_str(entry->data, &len);
    len += strlen(entry->name) + 1;
    if (entry->data && redis_push(data_entry_string(entry, len), len)) {
        fprintf(stdout, "[Info] Push succ.\n");
    } else {
        fprintf(stderr, "[Error] Push failed.\n");
    }
    return 0;
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

int producer_mongo(int argc, char **argv) {
    // ./producer_mongo 10.186.5.116 6379 "mongodb://12.11.70.12:10101" wega_data DrugBank
    if (init_producer_mongo(argc, argv)) {
        mongo_get();
    } else {
        fprintf(stderr, "[Error] init_producer_mongo failed\n");
    }
    destory_producer_mongo();
    return 0;
}

int producer(int argc, char **argv) {
    // ./producer 127.0.0.1:8080 zinc_datazinc_ligand_1w_sort ZINC21247520 ZINC21247520.pdbqt
    data_entry entry;
    if (init_producer(argc, argv, &entry)) {
        file_get(&entry);
    } else {
        fprintf(stderr, "[Error] init_producer failed\n");
    }
    destory_producer();
    return 0;
}

int consumer(int argc, char **argv) {
    // ./consumer 127.0.0.1:8080 zinc_datazinc_ligand_1w_sort
    if (init_cluster(argc, argv)) {
        redis_get();
    } else {
        fprintf(stderr, "[Error] init_cluster failed\n");
    }
    destory_cluster();
    return 0;
}

int main(int argc, char **argv) {
    if (strstr(argv[0], "consumer")) {
        consumer(argc, argv);
    } else if (strstr(argv[0], "mongo")) {
        producer_mongo(argc, argv);
    } else {
        producer(argc, argv);
    }
    return 0;
}

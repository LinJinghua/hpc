#include "recovery.h"
#include "redis_op.h"
#include "mongo_op.h"
#include "shell.h"
#include "vina.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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

int init_producer(int argc, char **argv, char** name, char** vina_file) {
    const char* schedule_addr = (argc > 1) ? argv[1] : "127.0.0.1:8080";
    const char* redis_queue = (argc > 2) ? argv[2] : "task";
    entry_id_check(redis_queue);
    *name = (argc > 3) ? argv[3] : "target";
    *vina_file = (argc > 4) ? argv[4] : "target.pdbqt";
    return redis_connect(host_get(schedule_addr), REDIS_PORT);
}

int init_score_query(int argc, char **argv, char** name) {
    const char* redis_queue = (argc > 3) ? argv[3] : "result";
    *name = (argc > 4) ? argv[4] : "ZINC00717479";
    entry_id_check(redis_queue);
    return redis_init(argc, argv);
}

int destory_score_query() {
    redis_destroy();
    return 0;
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

int write_result(const char* name, double score, const char* mol_file) {
    size_t len;
    char* data = file_str(mol_file, &len);
    return data && redis_record(name, score, data, len);
}

int file_get(const char* name, const char* vina_file) {
    size_t len = 0;
    data_entry entry;
    entry.name = name;
    entry.data = file_str(vina_file, &len);
    len += strlen(entry.name) + 1;
    return entry.data && redis_push(data_entry_string(&entry, len), len);
}

int redis_push_entry(data_entry* entry) {
    size_t len = strlen(entry->name) + 1 + strlen(entry->data);
    return redis_push(data_entry_string(entry, len), len);
}

int error_write_file(const char* name, data_entry* entry) {
    fprintf(stderr, "[Error] write_file failed\n");
    fprintf(stderr, "[Error] Run task %s failed\n", name);
    if (redis_push_entry(entry)) {
        fprintf(stdout, "[Info] Write back task %s succ\n", name);
    } else {
        fprintf(stderr, "[Error] Write back task %s failed\n", name);
    }
    return 0;
}

int error_run_file(const char* name, const char* vina_file) {
    fprintf(stderr, "[Error] Run task %s failed\n", name);
    if (file_get(name, vina_file)) {
        fprintf(stdout, "[Info] Write back task %s succ\n", name);
    } else {
        fprintf(stderr, "[Error] Write back task %s failed\n", name);
    }
    return 0;
}

int score_get(const char* name) {
    double score = redis_zscore(name);
    if (isnan(score)) {
        fprintf(stderr, "[Error] : Not find score of %s\n", name);
        return 0;
    }
    const char* data = redis_get_name(name);
    if (!data) {
        fprintf(stderr, "[Error] : Not find data of %s\n", name);
        return 0;
    }
    fprintf(stdout, "name: %s\nscore: %.15lf\ndata: %s\n", name, score, data);
    return 1;
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
        if (!write_file(vina_file, entry.data, strlen(entry.data))) {
            error_write_file(name, &entry);
            break;
        }
        if (vina_run(vina_file, pdbqt_file) == 0
            && obabel_run(pdbqt_file, mol_file) == 0
            && write_result(name, vina_score(pdbqt_file), mol_file)) {
            fprintf(stdout, "[Info] Run task %s succ\n", name);
        } else {
            error_run_file(name, vina_file);
        }
        if (is_interrupt()) {
            break;
        }
    }
    return 0;
}

int mongo_get() {
#ifdef TEST_REDIS_RW
    redis_get();
#endif // !TEST_REDIS_RW
    const int unit = 1000000;
    const int idxd_begin = mongo_idxd_get() * unit;
    const int idxd_end = idxd_begin + unit;
    fprintf(stdout, "[Info] idxd: (%d,%d]\n", idxd_begin, idxd_end);
    bson_t* query = BCON_NEW(("idxd"),
        "{", "$gt", BCON_INT32(idxd_begin), "$lte", BCON_INT32(idxd_end), "}");
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
#ifdef CHECK_RESULT
        if (redis_get_name(entry.name)) {
            continue;
        }
        fprintf(stdout, "[Info] Not result: %s\n", entry.name);
#endif // !CHECK_RESULT
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
    // ./pmongo 10.186.5.116 6379 "mongodb://12.11.70.12:10101" wega_data DrugBank
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
    char *name, *vina_file;
    if (init_producer(argc, argv, &name, &vina_file)) {
        if (file_get(name, vina_file)) {
            fprintf(stdout, "[Info] Push %s succ.\n", name);
        } else {
            fprintf(stderr, "[Error] Push %s failed.\n", name);
        }
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

int socre_query(int argc, char **argv) {
    // ./scorequery 127.0.0.1 6379 _zinc_datazinc_ligand_1w_sort ZINC00717479
    char* name = NULL;
    if (init_score_query(argc, argv, &name)) {
        score_get(name);
    }
    destory_score_query();
    return 0;
}

int main(int argc, char **argv) {
    flow_control_init();
    if (strstr(argv[0], "consumer")) {
        consumer(argc, argv);
    } else if (strstr(argv[0], "mongo")) {
        producer_mongo(argc, argv);
    } else if (strstr(argv[0], "query")) {
        socre_query(argc, argv);
    } else {
        producer(argc, argv);
    }
    return 0;
}

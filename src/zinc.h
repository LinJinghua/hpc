#ifndef ___ZINC_H___
#define ___ZINC_H___

#include <string.h>
#include <stdio.h>

#ifndef _ID_MAX_LEN
#define _ID_MAX_LEN (256)
#endif // !_ID_MAX_LEN
#ifndef _NAME_FIELD_MAX_LEN
#define _NAME_FIELD_MAX_LEN (256)
#endif // !_NAME_FIELD_MAX_LEN
#ifndef STRING_MAX_LEN
#define STRING_MAX_LEN (16 * 1024 * 1024)
#endif // !STRING_MAX_LEN

typedef struct entry_id {
    const char* name_field;
    const char* data_field;
    char id[_ID_MAX_LEN];
} entry_id;

static entry_id _entry_id;
static int _mongo_idxd;

int mongo_idxd_get() {
    return _mongo_idxd;
}

void mongo_idxd_set(int x) {
    _mongo_idxd = x;
}

const char* entry_id_get_name_field() {
    return _entry_id.name_field;
}

const char* entry_id_get_data_field() {
    return _entry_id.data_field;
}

void entry_id_set_field(const char* name_field, const char* data_field) {
    _entry_id.name_field = name_field;
    _entry_id.data_field = data_field;
}

const char* entry_id_get() {
    return _entry_id.id;
}

int entry_id_check(const char* p) {
    char *str = _entry_id.id, *end = _entry_id.id + sizeof(_entry_id.id) - 1;
    for (; *p; ++p) {
        if (*p != ' ') {
            *str = *p;
            if (++str >= end) {
                break;
            }
        }
    }
    *str = '\0';
    fprintf(stdout, "[Info] _id: `%s`\n",  entry_id_get());
    return 0;
}

int entry_id_set(const char *database_name, const char *collection_name) {
    size_t len_db = strlen(database_name), len_coll = strlen(collection_name);
    if (len_db >= sizeof(_entry_id.id)) {
        len_db = sizeof(_entry_id.id) - 1;
        len_coll = 0;
    } else if (len_db + len_coll > sizeof(_entry_id.id)) {
        len_coll = sizeof(_entry_id.id) - 1 - len_db;
    }
    _entry_id.id[len_db + len_coll] = '\0';
    memcpy(_entry_id.id, database_name, len_db);
    memcpy(_entry_id.id + len_db, collection_name, len_coll);

    return entry_id_check(_entry_id.id);
}

typedef struct data_entry {
    const char* name;
    const char* data;
} data_entry;

int data_entry_cmp(data_entry* a, data_entry* b) {
    return strcmp(a->name, b->name)
        || strcmp(a->data, b->data);
}

void data_entry_init(data_entry* entry) {
    entry->name = NULL;
    entry->data = NULL;
}

data_entry data_entry_get(const char* str) {
    data_entry entry;
    entry.name = str;
    entry.data = strchr(str, '\0') + 1;
    return entry;
}

static char* entrycpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return dest;
}

const char* data_entry_string(data_entry* entry, const unsigned int len) {
    static char buf[STRING_MAX_LEN];
    if (len > sizeof(buf) || !entry->name || !entry->data) {
        return NULL;
    }
    char* str = entrycpy(buf, entry->name);
#ifdef DEBUG_CHECK
    str = entrycpy(str + 1, entry->data);
    if (len != (unsigned)(str - buf)) {
        fprintf(stderr, "[Debug] %u but expect %u\n", (unsigned)(str - buf), len);
    }
#else
    entrycpy(str + 1, entry->data);
#endif // !DEBUG_CHECK
    return buf;
}


#endif // ! ___ZINC_H___

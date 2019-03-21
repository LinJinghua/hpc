#ifndef ___ZINC_H___
#define ___ZINC_H___

#include <string.h>
#include <stdio.h>

#ifndef STRING_MAX_LEN
#define STRING_MAX_LEN (16 * 1024 * 1024)
#endif // !STRING_MAX_LEN

typedef struct zinc_entry {
    const char* index_name;
    const char* pdbqt_file;
    char num_atoms[16];
} zinc_entry;

int zinc_entry_cmp(zinc_entry* a, zinc_entry* b) {
    return strcmp(a->index_name, b->index_name)
        || strcmp(a->pdbqt_file, b->pdbqt_file)
        || strcmp(a->num_atoms, b->num_atoms);
}

void zinc_entry_init(zinc_entry* entry) {
    entry->index_name = NULL;
    entry->pdbqt_file = NULL;
    entry->num_atoms[0] = '\0';
}

zinc_entry zinc_entry_get(const char* str) {
    zinc_entry entry;
    entry.index_name = str;
    entry.pdbqt_file = strchr(str, '\0') + 1;
    strcpy(entry.num_atoms, strchr(entry.pdbqt_file, '\0') + 1);
    return entry;
}

static char* strcpy_len(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return dest;
}

const char* zinc_entry_string(zinc_entry* entry, const unsigned int len) {
    static char buf[STRING_MAX_LEN];
    if (len > sizeof(buf) || !entry->index_name
        || !entry->pdbqt_file || !entry->num_atoms[0]) {
        return NULL;
    }
    char* str = strcpy_len(buf, entry->index_name);
    str = strcpy_len(str + 1, entry->pdbqt_file);
#ifdef DEBUG_CHECK
    str = strcpy_len(str + 1, entry->num_atoms);
    if (len != (unsigned)(str - buf)) {
        fprintf(stderr, "[Debug] %u but expect %u\n", (unsigned)(str - buf), len);
    }
#else
    strcpy_len(str + 1, entry->num_atoms);
#endif // !DEBUG_CHECK
    return buf;
}


#endif // ! ___ZINC_H___

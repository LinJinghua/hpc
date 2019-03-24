#ifndef ___FILE_OP_H___
#define ___FILE_OP_H___

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef CHECK_AND_RETURN
#define CHECK_AND_RETURN(condition, ret, fmt, ...) do {     \
    if (condition) {                                        \
        fprintf(stderr, fmt, ##__VA_ARGS__);                \
        return ret;                                         \
    }                                                       \
} while (0)
#endif // !CHECK_AND_RETURN
#ifndef CHECK_AND_JUMP
#define CHECK_AND_JUMP(condition, label, fmt, ...) do {     \
    if (condition) {                                        \
        fprintf(stderr, fmt, ##__VA_ARGS__);                \
        goto label;                                         \
    }                                                       \
} while (0)
#endif // !CHECK_AND_JUMP
#define CHECK_ERRNO_AND_JUMP(condition, label) do {         \
    CHECK_AND_JUMP(condition, label,                        \
        "[Error] get_file: %s", strerror(errno));           \
} while (0)

// [Correct way to read a text file into a buffer in C? [duplicate]]
// (https://stackoverflow.com/questions/2029103/correct-way-to-read-a-text-file-into-a-buffer-in-c)
char* get_file(const char* filename, size_t* len) {
    FILE* fp = fopen(filename, "r");
    CHECK_AND_RETURN(!fp, NULL, "[Error] get_file: %s", strerror(errno));

    char* source = NULL;
    CHECK_ERRNO_AND_JUMP(fseek(fp, 0L, SEEK_END), closefile);
    long bufsize = ftell(fp);
    CHECK_ERRNO_AND_JUMP(bufsize == -1, closefile);
    CHECK_ERRNO_AND_JUMP(fseek(fp, 0L, SEEK_SET), closefile);

    source = malloc(sizeof(char) * (bufsize + 1));
    *len = fread(source, sizeof(char), bufsize, fp);

    CHECK_ERRNO_AND_JUMP(ferror(fp), closefile);
closefile:
    fclose(fp);
    return source;
}

void free_file(char* data) {
    free(data);
}

int write_file(const char* filename, const void* bytes, size_t len) {
    FILE *fp = fopen(filename, "wb");
    CHECK_AND_RETURN(!fp, 0, "[Error] write_file: %s", strerror(errno));

    size_t size = fwrite(bytes, 1, len, fp);
    fclose(fp);
    return len == size? 1 : 0;
}

#endif // ! ___FILE_OP_H___

#ifndef ___FILE_OP_H___
#define ___FILE_OP_H___

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef STRING_MAX_LEN
#define STRING_MAX_LEN (16 * 1024 * 1024)
#endif // !STRING_MAX_LEN
#ifndef LINE_MAX_LEN
#define LINE_MAX_LEN (4096)
#endif // !LINE_MAX_LEN

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
        "[Error] %s: %s\n", __PRETTY_FUNCTION__,            \
        strerror(errno));                                   \
} while (0)

// [Correct way to read a text file into a buffer in C? [duplicate]]
// (https://stackoverflow.com/questions/2029103/correct-way-to-read-a-text-file-into-a-buffer-in-c)
char* get_file(const char* filename, size_t* len) {
    FILE* fp = fopen(filename, "r");
    CHECK_AND_RETURN(!fp, NULL, "[Error] get_file: %s\n", strerror(errno));

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

char* file_str(const char* filename, size_t* len) {
    static char _buf[STRING_MAX_LEN];
    FILE* fp = fopen(filename, "r");
    CHECK_AND_RETURN(!fp, NULL, "[Error] file_str: %s\n", strerror(errno));

    CHECK_ERRNO_AND_JUMP(fseek(fp, 0L, SEEK_END), closefile);
    long bufsize = ftell(fp);
    CHECK_AND_JUMP(bufsize >= STRING_MAX_LEN, closefile,
        "[Error] file_str buffer < file");
    CHECK_ERRNO_AND_JUMP(bufsize == -1, closefile);
    CHECK_ERRNO_AND_JUMP(fseek(fp, 0L, SEEK_SET), closefile);

    *len = fread(_buf, sizeof(char), bufsize, fp);

    CHECK_ERRNO_AND_JUMP(ferror(fp), closefile);

    _buf[*len] = '\0';
    fclose(fp);
    return _buf;
closefile:
    fclose(fp);
    return NULL;
}

char* file_line(const char* filename, size_t line) {
    static char _buf[LINE_MAX_LEN];
    FILE* fp = fopen(filename, "r");
    CHECK_AND_RETURN(!fp, NULL, "[Error] file_line: %s\n", strerror(errno));

    for (; line > 0; --line) {
        if (!fgets(_buf, sizeof(_buf), fp)) {
            CHECK_AND_JUMP(feof(fp), closefile,
                "[Error] End of file(%s) reached(need more %lu lines)\n",
                filename, line);
            CHECK_ERRNO_AND_JUMP(ferror(fp), closefile);
        }
    }

    fclose(fp);
    return _buf;
closefile:
    fclose(fp);
    return NULL;
}

int write_file(const char* filename, const void* bytes, size_t len) {
    FILE *fp = fopen(filename, "wb");
    CHECK_AND_RETURN(!fp, 0, "[Error] write_file: %s\n", strerror(errno));

    size_t size = fwrite(bytes, 1, len, fp);
    fclose(fp);
    return len == size? 1 : 0;
}

#endif // ! ___FILE_OP_H___

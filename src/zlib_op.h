#ifndef ___ZLIB_OP_H___
#define ___ZLIB_OP_H___

#include "file_op.h"

#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>

#define CHECK_ZLIB_ERR(err, msg) do {                       \
    if (err != Z_OK) {                                      \
        fprintf(stderr, "[Error] %s: %d\n", msg, err);      \
        exit(109);                                          \
    }                                                       \
} while (0)

#ifndef STRING_MAX_LEN
#define STRING_MAX_LEN (16 * 1024 * 1024)
#endif // !STRING_MAX_LEN

static char _zlib_buf[STRING_MAX_LEN];

const char* compress_bytes(const void* bytes, unsigned long* bytes_len_p) {
    uLong len = sizeof(_zlib_buf), bytes_len = *bytes_len_p;
    int err = compress((Byte*)_zlib_buf, &len, (const Bytef*)bytes, bytes_len);
    CHECK_ZLIB_ERR(err, "compress");
    *bytes_len_p = len;
    return _zlib_buf;
}

const char* uncompress_bytes(const void* bytes, unsigned long* bytes_len_p) {
    uLong len = sizeof(_zlib_buf), bytes_len = *bytes_len_p;
    int err = uncompress((Byte*)_zlib_buf, &len, (const Bytef*)bytes, bytes_len);
    CHECK_ZLIB_ERR(err, "uncompress");
    _zlib_buf[len] = '\0';
    *bytes_len_p = len;
    return _zlib_buf;
}

void print_compress(const void* bytes, unsigned long len) {
    unsigned long comp_len = len;
    compress_bytes(bytes, &comp_len);
    printf("[compress] %lf,%lu/%lu\n", (double)comp_len / len, comp_len, len);
}

int test_compress(int argc, char **argv) {
    if (argc < 2) {
        return 1;
    }
    size_t len;
    char* str = file_str(argv[1], &len);
    if (str) {
        print_compress(str, len);
    }
    return 0;
}

#endif // ! ___ZLIB_OP_H___

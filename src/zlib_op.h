#ifndef ___ZLIB_OP_H___
#define ___ZLIB_OP_H___

#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>

#define CHECK_ZLIB_ERR(err, msg) do {                       \
    if (err != Z_OK) {                                      \
        fprintf(stderr, "[Error] %s: %d\n", msg, err);      \
        exit(109);                                          \
    }                                                       \
} while (0)
#define ZLIB_MAX_LEN (16 * 1024 * 1024)

static char _zlib_buf[ZLIB_MAX_LEN];

const char* compress_bytes(const void* bytes, unsigned long* bytes_len_p) {
    uLong len = ZLIB_MAX_LEN, bytes_len = *bytes_len_p;
    int err = compress((Byte*)_zlib_buf, &len, (const Bytef*)bytes, bytes_len);
    CHECK_ZLIB_ERR(err, "compress");
    *bytes_len_p = len;
    return _zlib_buf;
}

const char* uncompress_bytes(const void* bytes, unsigned long* bytes_len_p) {
    uLong len = ZLIB_MAX_LEN, bytes_len = *bytes_len_p;
    int err = uncompress((Byte*)_zlib_buf, &len, (const Bytef*)bytes, bytes_len);
    CHECK_ZLIB_ERR(err, "uncompress");
    _zlib_buf[len] = '\0';
    *bytes_len_p = len;
    return _zlib_buf;
}

#endif // ! ___ZLIB_OP_H___

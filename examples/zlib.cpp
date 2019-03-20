/**
 * [zlib/test/example.c](https://github.com/madler/zlib/blob/master/test/example.c)
 * [Compress and then Decompress a string with zlib. ](https://gist.github.com/arq5x/5315739)
 * g++ zlib.cpp ./bin/lib/libz.a -o t -I ./bin/include/ && ./t
 * g++ examples/zlib.cpp lib/libz.a -I include/ -o t && ./t
  */

#include <stdlib.h> // for exit
#include <stdio.h>
#include <string.h>  // for strlen
#include <assert.h>
#include "zlib.h"

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

// adapted from: http://stackoverflow.com/questions/7540259/deflate-and-inflate-zlib-h-in-c
int main(int argc, char* argv[])
{   
    // original string len = 36
    char a[50] = "Hello Hello Hello Hello Hello Hello!"; 

    // placeholder for the compressed (deflated) version of "a" 
    char b[50];

    // placeholder for the UNcompressed (inflated) version of "b"
    char c[50];
    
    const int LEN = 4096;
    char compr_[LEN] = {0}, uncompr_[LEN] = {0};
    Byte *compr = (Byte*)compr_, *uncompr = (Byte*)uncompr_;
    uLong comprLen = sizeof(compr_), uncomprLen = sizeof(uncompr_);
    int err;
    uLong len = (uLong)strlen(a) + 1;    
    err = compress(compr, &comprLen, (const Bytef*)a, len);
    CHECK_ERR(err, "compress");
    printf("[zlib] comprLen:%lu '%s'\n\t len:%lu '%s'\n",
        comprLen, compr_, len, a);
    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");
    printf("[zlib] uncomprLen:%lu '%s'\n\t comprLen:%lu '%s'\n",
        uncomprLen, uncompr_, comprLen, compr_);
    
    

    printf("Uncompressed size is: %lu\n", strlen(a) + 1);
    printf("Uncompressed string is: %s\n", a);


    printf("\n----------\n\n");

    // STEP 1.
    // deflate a into b. (that is, compress a into b)
    
    // zlib struct
    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;
    // setup "a" as the input and "b" as the compressed output
    defstream.avail_in = (uInt)strlen(a)+1; // size of input, string + terminator
    defstream.next_in = (Bytef *)a; // input char array
    defstream.avail_out = (uInt)sizeof(b); // size of output
    defstream.next_out = (Bytef *)b; // output char array
    
    // the actual compression work.
    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);

    // This is one way of getting the size of the output
    printf("Compressed size is: %lu\n", defstream.total_out);
    printf("Compressed string is: %s\n", b);
    

    printf("\n----------\n\n");


    // STEP 2.
    // inflate b into c
    // zlib struct
    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
    // setup "b" as the input and "c" as the compressed output
    infstream.avail_in = (uInt)((char*)defstream.next_out - b); // size of input
    infstream.next_in = (Bytef *)b; // input char array
    infstream.avail_out = (uInt)sizeof(c); // size of output
    infstream.next_out = (Bytef *)c; // output char array
     
    // the actual DE-compression work.
    inflateInit(&infstream);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);
     
    printf("Uncompressed size is: %lu\n", strlen(c) + 1);
    printf("Uncompressed string is: %s\n", c);
    

    // make sure uncompressed is exactly equal to original.
    assert(strcmp(a,c)==0);

    return 0;
}


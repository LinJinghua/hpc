#ifndef ___BENCHMARK_H___
#define ___BENCHMARK_H___

#include "redis_op.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef BENCHMARK_NUMBER
#define BENCHMARK_NUMBER 500
#endif // !BENCHMARK_NUMBER

#define TIME_MEASURE_BEGIN double __time = get_astronomical_time()
#define TIME_MEASURE_END(info) fprintf(stdout,                      \
    "[Time] %s %.6lf\n", info, get_astronomical_time() - __time)

double get_astronomical_time() {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec + now.tv_nsec * 1e-9;
}

int redis_get_rw_benchmark() {
    int test = BENCHMARK_NUMBER;
    fprintf(stdout, "[Test] %s times: %d\n", __PRETTY_FUNCTION__, test);
    TIME_MEASURE_BEGIN;
    for (const char* str = redis_pop(); --test >= 0 && str; str = redis_pop()) {
        unsigned long len = strchr(strchr(str, '\0') + 1, '\0') - str;
        redis_push(str, len);
    }
    TIME_MEASURE_END(__PRETTY_FUNCTION__);
    return 0;
}

int redis_get_read_benchmark() {
    int test = BENCHMARK_NUMBER;
    fprintf(stdout, "[Test] %s times: %d\n", __PRETTY_FUNCTION__, test);
    TIME_MEASURE_BEGIN;
    for (const char* str = redis_pop(); --test >= 0 && str; str = redis_pop()) {
    }
    TIME_MEASURE_END(__PRETTY_FUNCTION__);
    return 0;
}

int redis_get_write_benchmark() {
    int test = BENCHMARK_NUMBER;
    fprintf(stdout, "[Test] %s times: %d\n", __PRETTY_FUNCTION__, test);
    const char* str = redis_pop();
    if (str) {
        TIME_MEASURE_BEGIN;
        unsigned long len = strchr(strchr(str, '\0') + 1, '\0') - str;
        while (--test >= 0) {
            redis_push(str, len);
        }
        TIME_MEASURE_END(__PRETTY_FUNCTION__);
    }
    return 0;
}

#endif // ! ___BENCHMARK_H___

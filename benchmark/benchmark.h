#ifndef BENCHMARK_H
#define BENCHMARK_H

typedef struct {
    char *data;
    size_t size;
} bstring_t;

double benchmark_get_time(void);

#define BENCHMARK_ERR_NO_FILE              -1
#define BENCHMARK_ERR_NO_STRING            -2
#define BENCHMARK_ERR_FAILED_TO_OPEN_FILE  -3
#define BENCHMARK_ERR_FAILED_TO_SEEK_FILE  -4
#define BENCHMARK_ERR_FAILED_TO_FTELL_FILE -5
#define BENCHMARK_ERR_OUT_OF_MEMORY        -6
#define BENCHMARK_ERR_FAILED_TO_READ_FILE  -7

int benchmark_load_fixture(const char *filepath, bstring_t *string);
void benchmark_free_fixture(bstring_t *string);

void benchmark_report(const char *name, double *measures, size_t size);

#endif

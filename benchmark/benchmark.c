#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "benchmark.h"

#ifdef WIN32

#include <windows.h>
double benchmark_get_time(void)
{
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return (double)t.QuadPart/(double)f.QuadPart;
}

#else

#include <sys/time.h>
#include <sys/resource.h>

double benchmark_get_time(void)
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
}

#endif

int benchmark_load_fixture(const char *filepath, bstring_t *string)
{
    if (!filepath) return BENCHMARK_ERR_NO_FILE;
    if (!string) return BENCHMARK_ERR_NO_STRING;

    FILE *file = fopen(filepath, "r");
    if (!file) return BENCHMARK_ERR_FAILED_TO_OPEN_FILE;

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return BENCHMARK_ERR_FAILED_TO_SEEK_FILE;
    }

    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return BENCHMARK_ERR_FAILED_TO_FTELL_FILE;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return BENCHMARK_ERR_FAILED_TO_SEEK_FILE;
    }

    string->data = (char *)malloc(size + 1);
    if (!string->data) {
        fclose(file);
        return BENCHMARK_ERR_OUT_OF_MEMORY;
    }

    ssize_t read = fread(string->data, 1, size, file);
    if (read != size) {
        free(string->data);
        fclose(file);
        return BENCHMARK_ERR_FAILED_TO_READ_FILE;
    }

    string->data[size] = '\0'; // Null-terminate the string
    string->size = size;
    fclose(file);

    return 0;
}

void benchmark_free_fixture(bstring_t *string)
{
    if (string == NULL) {
        return;
    }

    if (string->data) {
        free(string->data);
        string->data = NULL;
    }
    string->size = 0;
}

double benchmark_stddev(double *measures, size_t size, double avg)
{
    if (measures == NULL || size == 0) {
        return 0.0;
    }

    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        double diff = measures[i] - avg;
        sum += diff * diff;
    }
    return sqrt(sum / size);
}

int compare_doubles(const void *a, const void *b)
{
    double diff = *(double *)a - *(double *)b;
    return (diff > 0) - (diff < 0);
}

double benchmark_median(double *measures, size_t size)
{
    if (measures == NULL || size == 0) {
        return 0.0;
    }

    // We dont need to copy, we could just sort the original
    // TODO: check if this is a problem
    double *sorted = malloc(size * sizeof(double));
    if (sorted == NULL) {
        return 0.0;
    }

    for (size_t i = 0; i < size; i++) {
        sorted[i] = measures[i];
    }

    // Sort the array
    qsort(sorted, size, sizeof(double), compare_doubles);

    double median;
    if (size % 2 == 0) {
        median = (sorted[size / 2 - 1] + sorted[size / 2]) / 2.0;
    } else {
        median = sorted[size / 2];
    }

    free(sorted);
    return median;
}

void benchmark_report(const char *name, double *measures, size_t size)
{
    if (name == NULL || measures == NULL || size == 0) {
        return;
    }

    printf("--------------------------------------------------\n");
    printf("Benchmarking %s...\n", name);
    printf("Number of samples: %zu\n", size);

    double sum = 0.0;
    double min = measures[0];
    double max = measures[0];

    for (size_t i = 0; i < size; i++) {
        sum += measures[i];
        if (measures[i] < min) {
            min = measures[i];
        }
        if (measures[i] > max) {
            max = measures[i];
        }
    }

    double avg = sum / size;

    printf("Benchmark: %s\n", name);
    printf("Average: %.6f seconds\n", avg);
    printf("Median: %.6f seconds\n", benchmark_median(measures, size));
    printf("Minimum: %.6f seconds\n", min);
    printf("Maximum: %.6f seconds\n", max);
    printf("Standard Deviation: %.6f seconds\n", benchmark_stddev(measures, size, avg));
    printf("--------------------------------------------------\n");
}

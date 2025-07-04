#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "benchmark.h"

#ifdef WIN32

#error "Windows is not supported yet. Please use Linux or MacOS."

#else

#include <sys/time.h>
#include <sys/resource.h>

double benchmark_get_time(void)
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec * 1e-6;
}

#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

double benchmark_stddev(double *measures, size_t size, double avg)
{
    if (measures == NULL || size == 0)
    {
        return 0.0;
    }

    double sum = 0.0;
    for (size_t i = 0; i < size; i++)
    {
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
    if (measures == NULL || size == 0)
    {
        return 0.0;
    }

    // We dont need to copy, we could just sort the original
    // TODO: check if this is a problem
    double *sorted = malloc(size * sizeof(double));
    if (sorted == NULL)
    {
        return 0.0;
    }

    for (size_t i = 0; i < size; i++)
    {
        sorted[i] = measures[i];
    }

    // Sort the array
    qsort(sorted, size, sizeof(double), compare_doubles);

    double median;
    if (size % 2 == 0)
    {
        median = (sorted[size / 2 - 1] + sorted[size / 2]) / 2.0;
    }
    else
    {
        median = sorted[size / 2];
    }

    free(sorted);
    return median;
}

void benchmark_report(char *name, double *measures, size_t size)
{
    if (name == NULL || measures == NULL || size == 0)
    {
        return;
    }

    printf("--------------------------------------------------\n");
    printf("Benchmarking %s...\n", name);
    printf("Number of samples: %zu\n", size);

    double sum = 0.0;
    double min = measures[0];
    double max = measures[0];

    for (size_t i = 0; i < size; i++)
    {
        sum += measures[i];
        min = MIN(min, measures[i]);
        max = MAX(max, measures[i]);
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

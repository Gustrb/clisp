#ifndef BENCHMARK_H
#define BENCHMARK_H

double benchmark_get_time(void);
void benchmark_report(char *name, double *measures, size_t size);

#endif

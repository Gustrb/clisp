#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "parser.h"
#include "benchmark.h"

#define SAMPLE_SIZE 20
static double measures[SAMPLE_SIZE];

void benchmark_it(const char *path)
{
    io_str_t string;
    int err = io_load_file_into_memory(path, &string);
    if (err) {
        fprintf(stderr, "Error loading fixture: %d\n", err);
        return;
    }

    for (size_t i = 0; i < SAMPLE_SIZE; i++) {
        double start = benchmark_get_time();
        parser_t parser;
        program_t program = {0};
        err = parser_init(&parser, string.data, string.size);
        err = parser_parse(&parser, &program);
        if (err) {
            fprintf(stderr, "Error parsing: %d\n", err);
            continue;
        }

        double end = benchmark_get_time();
        measures[i] = end - start;
        parser_free_program(&program);
    }

    io_free_string(&string);
    benchmark_report(path, measures, SAMPLE_SIZE);
}

int main(void)
{
    printf("Parser Benchmark\n");

    benchmark_it("./benchmark/fixtures/small.lisp");
    benchmark_it("./benchmark/fixtures/medium.lisp");
    benchmark_it("./benchmark/fixtures/large.lisp");

    printf("Parser Benchmark Complete\n");

    return 0;
}

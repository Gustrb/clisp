#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "lexer.h"
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

    lexer_t l;

    for (size_t i = 0; i < SAMPLE_SIZE; i++) {
        double start = benchmark_get_time();
        lexer_init(&l, string.data, string.size);
        token_t token;
        while (1) {
            err = lexer_next_token(&l, &token);
            if (err) {
                fprintf(stderr, "Error lexing: %d\n", err);
                break;
            }

            if (token.type == TOK_EOF) break;
        }
        double end = benchmark_get_time();
        measures[i] = end - start;
    }

    io_free_string(&string);
    benchmark_report(path, measures, SAMPLE_SIZE);
}

int main(void)
{
    printf("Lexer Benchmark\n");

    benchmark_it("./benchmark/fixtures/small.lisp");
    benchmark_it("./benchmark/fixtures/medium.lisp");
    benchmark_it("./benchmark/fixtures/large.lisp");

    printf("Lexer Benchmark Complete\n");

    return 0;
}

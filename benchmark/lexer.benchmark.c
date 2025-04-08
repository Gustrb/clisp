#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "benchmark.h"

#define SAMPLE_SIZE 1024

static double measures[SAMPLE_SIZE];

void benchmark_it(const char *path)
{
    string_t string;
    int err = benchmark_load_fixture(path, &string);
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
            lexer_next_token(&l, &token);
            if (token.type == TOK_EOF) break;
        }
        double end = benchmark_get_time();
        measures[i] = end - start;
    }

    benchmark_free_fixture(&string);
    benchmark_report(path, measures, SAMPLE_SIZE);
}

int main(void)
{
    printf("Lexer Benchmark\n");

    benchmark_it("./benchmark/fixtures/code.small.lisp");
    benchmark_it("./benchmark/fixtures/code.medium.lisp");
    benchmark_it("./benchmark/fixtures/code.large.lisp");

    printf("Lexer Benchmark Complete\n");

    return 0;
}

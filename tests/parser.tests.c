#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "parser.h"

int should_parse_integer_atom(void);
int should_parse_float_atom(void);
int should_fail_on_invalid_number(void);

int main(void) {
    int err = 0;
    err = err || should_parse_integer_atom();
    err = err || should_parse_float_atom();
    err = err || should_fail_on_invalid_number();

    if (err == 0) {
        printf("[OK] All parser tests passed\n");
    } else {
        printf("[FAIL] Some parser tests failed\n");
    }
    return err;
}

int should_parse_integer_atom(void) {
    fprintf(stdout, "[TEST] should_parse_integer_atom\n");
    parser_t parser;
    program_t program = {0};
    const char *input = "12345";
    int err = parser_init(&parser, input, strlen(input));
    if (err) return 1;
    err = parser_parse(&parser, &program);
    if (err) {
        fprintf(stderr, "[FAIL] parser_parse failed: %d\n", err);
        return 1;
    }
    if (program.size != 1 || program.items[0].type != FORM_ATOM ||
        program.items[0].atom.type != ATOM_NUMBER ||
        program.items[0].atom.num.type != NUMBER_INTEGER ||
        program.items[0].atom.num.integer != 12345) {
        fprintf(stderr, "[FAIL] should_parse_integer_atom: incorrect parse result\n");
        return 1;
    }
    fprintf(stdout, "[PASS] should_parse_integer_atom\n");
    return 0;
}

int should_parse_float_atom(void) {
    fprintf(stdout, "[TEST] should_parse_float_atom\n");
    parser_t parser;
    program_t program = {0};
    const char *input = "3.14159";
    int err = parser_init(&parser, input, strlen(input));
    if (err) return 1;
    err = parser_parse(&parser, &program);
    if (err) {
        fprintf(stderr, "[FAIL] parser_parse failed: %d\n", err);
        return 1;
    }
    if (program.size != 1 || program.items[0].type != FORM_ATOM ||
        program.items[0].atom.type != ATOM_NUMBER ||
        program.items[0].atom.num.type != NUMBER_FLOAT ||
        program.items[0].atom.num.float_num < 3.14158 ||
        program.items[0].atom.num.float_num > 3.14160) {
        fprintf(stderr, "[FAIL] should_parse_float_atom: incorrect parse result\n");
        return 1;
    }
    fprintf(stdout, "[PASS] should_parse_float_atom\n");
    return 0;
}

int should_fail_on_invalid_number(void) {
    fprintf(stdout, "[TEST] should_fail_on_invalid_number\n");
    parser_t parser;
    program_t program = {0};
    const char *input = "notanumber";
    int err = parser_init(&parser, input, strlen(input));
    if (err) return 1;
    err = parser_parse(&parser, &program);
    if (err == 0) {
        fprintf(stderr, "[FAIL] should_fail_on_invalid_number: expected failure, got success\n");
        return 1;
    }
    fprintf(stdout, "[PASS] should_fail_on_invalid_number\n");
    return 0;
}

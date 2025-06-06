#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "parser.h"

int should_parse_integer_atom(void);
int should_fail_to_parse_integer_atom_that_is_too_large(void);
int should_parse_float_atom(void);
int should_fail_to_parse_float_atom_that_is_too_large(void);
int should_parse_symbol_atom(void);
int should_parse_string_atom(void);
int should_parse_list_of_atoms(void);
int should_parse_list_of_lists(void);
int should_parse_a_simple_lisp_program(void);
int should_fail_to_parse_unterminated_string_literal(void);
int should_fail_to_parse_unbalanced_parentheses(void);
int should_parse_multiple_top_level_forms(void);
int should_parse_nested_function_calls(void);
int should_parse_empty_list(void);
int should_fail_to_parse_unfinished_lists(void);
int should_parse_a_mathematical_expression(void);

int main(void)
{
    int err = 0;
    err = err || should_parse_integer_atom();
    err = err || should_fail_to_parse_integer_atom_that_is_too_large();
    err = err || should_parse_float_atom();
    err = err || should_fail_to_parse_float_atom_that_is_too_large();
    err = err || should_parse_symbol_atom();
    err = err || should_parse_string_atom();
    err = err || should_parse_list_of_atoms();
    err = err || should_parse_list_of_lists();
    err = err || should_parse_a_simple_lisp_program();
    err = err || should_fail_to_parse_unterminated_string_literal();
    err = err || should_fail_to_parse_unbalanced_parentheses();
    err = err || should_parse_multiple_top_level_forms();
    err = err || should_parse_nested_function_calls();
    err = err || should_parse_empty_list();
    err = err || should_fail_to_parse_unfinished_lists();
    err = err || should_parse_a_mathematical_expression();

    if (err == 0)
    {
        printf("[OK] All parser tests passed\n");
    }
    else
    {
        printf("[FAIL] Some parser tests failed\n");
    }
    return err;
}

int should_parse_integer_atom(void)
{
    fprintf(stdout, "[TEST] should_parse_integer_atom\n");
    parser_t parser;
    program_t program = {0};
    char *input = "12345";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_parse failed: %d\n", err);
        return 1;
    }
    if (program.size != 1 || program.items[0].type != FORM_ATOM ||
        program.items[0].atom.type != ATOM_NUMBER ||
        program.items[0].atom.num.type != NUMBER_INTEGER ||
        program.items[0].atom.num.integer != 12345)
    {
        fprintf(stderr, "[FAIL] should_parse_integer_atom: incorrect parse result\n");
        return 1;
    }
    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_free_program failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_parse_integer_atom\n");
    return 0;
}

int should_fail_to_parse_integer_atom_that_is_too_large(void)
{
    fprintf(stdout, "[TEST] should_fail_to_parse_integer_atom_that_is_too_large\n");
    parser_t parser;
    program_t program = {0};
    char *input = "1234567890123456789012345678901234567890";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err != PARSER_ERR_NUMBER_TOO_LARGE)
    {
        fprintf(stderr, "[FAIL] should_fail_to_parse_integer_atom_that_is_too_large: expected error %d, got %d\n", PARSER_ERR_NUMBER_TOO_LARGE, err);
        return 1;
    }
    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_free_program failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_fail_to_parse_integer_atom_that_is_too_large\n");
    return 0;
}

int should_parse_float_atom(void)
{
    fprintf(stdout, "[TEST] should_parse_float_atom\n");
    parser_t parser;
    program_t program = {0};
    char *input = "123.45";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_parse failed: %d\n", err);
        return 1;
    }
    if (program.size != 1 || program.items[0].type != FORM_ATOM ||
        program.items[0].atom.type != ATOM_NUMBER ||
        program.items[0].atom.num.type != NUMBER_FLOAT ||
        program.items[0].atom.num.float_num != 123.45)
    {
        fprintf(stderr, "[FAIL] should_parse_float_atom: incorrect parse result\n");
        return 1;
    }
    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_free_program failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_parse_float_atom\n");
    return 0;
}

int should_fail_to_parse_float_atom_that_is_too_large(void)
{
    fprintf(stdout, "[TEST] should_fail_to_parse_float_atom_that_is_too_large\n");
    parser_t parser;
    program_t program = {0};
    char *input = "1234567890123456789012345678901234567890.12345678901234567890";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err != PARSER_ERR_NUMBER_TOO_LARGE)
    {
        fprintf(stderr, "[FAIL] should_fail_to_parse_float_atom_that_is_too_large: expected error %d, got %d\n", PARSER_ERR_NUMBER_TOO_LARGE, err);
        return 1;
    }
    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_free_program failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_fail_to_parse_float_atom_that_is_too_large\n");
    return 0;
}

int should_parse_symbol_atom(void)
{
    fprintf(stdout, "[TEST] should_parse_symbol_atom\n");
    parser_t parser;
    program_t program = {0};
    char *input = "my_symbol";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_parse failed: %d\n", err);
        return 1;
    }
    if (program.size != 1 || program.items[0].type != FORM_ATOM ||
        program.items[0].atom.type != ATOM_SYMBOL ||
        strncmp(program.items[0].atom.sym.chars, "my_symbol", program.items[0].atom.sym.len) != 0)
    {
        fprintf(stderr, "[FAIL] should_parse_symbol_atom: incorrect parse result\n");
        return 1;
    }
    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_free_program failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_parse_symbol_atom\n");
    return 0;
}

int should_parse_string_atom(void)
{
    fprintf(stdout, "[TEST] should_parse_string_atom\n");
    parser_t parser;
    program_t program = {0};
    char *input = "\"my_string\"";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_parse failed: %d\n", err);
        return 1;
    }
    if (program.size != 1 || program.items[0].type != FORM_ATOM ||
        program.items[0].atom.type != ATOM_STRING ||
        strncmp(program.items[0].atom.str.chars, "\"my_string\"", program.items[0].atom.str.len) != 0)
    {
        fprintf(stderr, "[FAIL] should_parse_string_atom: incorrect parse result\n");
        return 1;
    }
    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_free_program failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_parse_string_atom\n");
    return 0;
}

int should_parse_list_of_atoms(void)
{
    fprintf(stdout, "[TEST] should_parse_list_of_atoms\n");
    parser_t parser;
    program_t program = {0};
    char *input = "(1 2.5 \"hello\" my_symbol)";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_parse failed: %d\n", err);
        return 1;
    }
    if (program.size != 1 || program.items[0].type != FORM_LIST ||
        program.items[0].list.size != 4)
    {
        fprintf(stderr, "[FAIL] should_parse_list_of_atoms: incorrect parse result\n");
        return 1;
    }
    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_free_program failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_parse_list_of_atoms\n");
    return 0;
}

int should_parse_list_of_lists(void)
{
    fprintf(stdout, "[TEST] should_parse_list_of_lists\n");
    parser_t parser;
    program_t program = {0};
    char *input = "((1 2) (3.5 4.5))";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_parse failed: %d\n", err);
        return 1;
    }
    if (program.size != 1 || program.items[0].type != FORM_LIST ||
        program.items[0].list.size != 2)
    {
        fprintf(stderr, "[FAIL] should_parse_list_of_lists: incorrect parse result\n");
        return 1;
    }
    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_free_program failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_parse_list_of_lists\n");
    return 0;
}

int should_parse_a_simple_lisp_program(void)
{
    fprintf(stdout, "[TEST] should_parse_a_simple_lisp_program\n");
    parser_t parser;
    program_t program = {0};
    char *input = "(define (factorial n) (if (= n 0) 1 (* n (factorial (- n 1)))))";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_parse failed: %d\n", err);
        return 1;
    }

    if (program.size != 1)
    {
        fprintf(stderr, "[FAIL] Expected 1 top-level form, got %zu\n", program.size);
        return 1;
    }

    form_t *top_form = &program.items[0];
    if (top_form->type != FORM_LIST)
    {
        fprintf(stderr, "[FAIL] Expected top-level form to be a list\n");
        return 1;
    }

    list_t *top_list = &top_form->list;
    if (top_list->size < 2)
    {
        fprintf(stderr, "[FAIL] Expected list with at least 2 elements, got %zu\n", top_list->size);
        return 1;
    }

    form_t *first = &top_list->items[0];
    if (first->type != FORM_ATOM || first->atom.type != ATOM_SYMBOL)
    {
        fprintf(stderr, "[FAIL] First item should be a symbol (define)\n");
        return 1;
    }

    if (strncmp(first->atom.sym.chars, "define", first->atom.sym.len) != 0)
    {
        fprintf(stderr, "[FAIL] First symbol should be 'define'\n");
        return 1;
    }

    form_t *second = &top_list->items[1];
    if (second->type != FORM_LIST)
    {
        fprintf(stderr, "[FAIL] Second item should be a list (function signature)\n");
        return 1;
    }

    list_t *signature = &second->list;
    if (signature->size < 2)
    {
        fprintf(stderr, "[FAIL] Signature list should have at least function name and one arg\n");
        return 1;
    }

    form_t *func_name = &signature->items[0];
    if (func_name->type != FORM_ATOM || func_name->atom.type != ATOM_SYMBOL)
    {
        fprintf(stderr, "[FAIL] Function name in signature should be a symbol\n");
        return 1;
    }

    if (strncmp(func_name->atom.sym.chars, "factorial", func_name->atom.sym.len) != 0)
    {
        fprintf(stderr, "[FAIL] Function name should be 'factorial'\n");
        return 1;
    }

    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_free_program failed: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_parse_a_simple_lisp_program\n");
    return 0;
}

int should_fail_to_parse_unterminated_string_literal(void)
{
    fprintf(stdout, "[TEST] should_fail_to_parse_unterminated_string_literal\n");
    parser_t parser;
    program_t program = {0};
    char *input = "(\"unterminated string)";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;

    err = parser_parse(&parser, &program);
    if (err != LEXER_ERR_UNTERMINATED_STRING_LITERAL)
    {
        fprintf(stderr, "[FAIL] should_fail_to_parse_unterminated_string_literal: expected error %d, got %d\n", LEXER_ERR_UNTERMINATED_STRING_LITERAL, err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_fail_to_parse_unterminated_string_literal\n");
    return 0;
}

int should_fail_to_parse_unbalanced_parentheses(void)
{
    fprintf(stdout, "[TEST] should_fail_to_parse_unbalanced_parentheses\n");
    parser_t parser;
    program_t program = {0};
    char *input = "(1 (2 3) (4 5)";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;

    err = parser_parse(&parser, &program);
    if (err != PARSER_ERR_UNEXPECTED_EOF)
    {
        fprintf(stderr, "[FAIL] should_fail_to_parse_unbalanced_parentheses: expected error %d, got %d\n", PARSER_ERR_UNEXPECTED_EOF, err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_fail_to_parse_unbalanced_parentheses\n");
    return 0;
}

int should_parse_multiple_top_level_forms(void)
{
    fprintf(stdout, "[TEST] should_parse_multiple_top_level_forms\n");
    parser_t parser;
    program_t program = {0};
    char *input = "(define x 1) (define y 2)";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] parser_parse failed: %d\n", err);
        return 1;
    }
    if (program.size != 2)
    {
        fprintf(stderr, "[FAIL] Expected 2 top-level forms, got %zu\n", program.size);
        return 1;
    }
    parser_free_program(&program);
    fprintf(stdout, "[PASS] should_parse_multiple_top_level_forms\n");
    return 0;
}

int should_parse_nested_function_calls(void)
{
    fprintf(stdout, "[TEST] should_parse_nested_function_calls\n");
    parser_t parser;
    program_t program = {0};
    char *input = "(f (g (h 1)))";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err || program.size != 1 || program.items[0].type != FORM_LIST)
    {
        fprintf(stderr, "[FAIL] parser_parse failed or incorrect form structure\n");
        return 1;
    }

    list_t *outer = &program.items[0].list;
    if (outer->size != 2 || outer->items[1].type != FORM_LIST)
    {
        fprintf(stderr, "[FAIL] Expected nested function structure\n");
        return 1;
    }

    parser_free_program(&program);
    fprintf(stdout, "[PASS] should_parse_nested_function_calls\n");
    return 0;
}

int should_parse_empty_list(void)
{
    fprintf(stdout, "[TEST] should_parse_empty_list\n");
    parser_t parser;
    program_t program = {0};
    char *input = "()";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;
    err = parser_parse(&parser, &program);
    if (err || program.size != 1 || program.items[0].type != FORM_LIST || program.items[0].list.size != 0)
    {
        fprintf(stderr, "[FAIL] Failed to parse empty list\n");
        return 1;
    }
    parser_free_program(&program);
    fprintf(stdout, "[PASS] should_parse_empty_list\n");
    return 0;
}

int should_fail_to_parse_unfinished_lists(void)
{
    fprintf(stdout, "[TEST] should_fail_to_parse_unfinished_lists\n");
    parser_t parser;
    program_t program = {0};
    char *input = "(1 (2 3) (4 5";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
        return 1;

    err = parser_parse(&parser, &program);
    if (err != PARSER_ERR_UNEXPECTED_EOF)
    {
        fprintf(stderr, "[FAIL] should_fail_to_parse_unfinished_lists: expected error %d, got %d\n", PARSER_ERR_UNEXPECTED_EOF, err);
        return 1;
    }

    fprintf(stdout, "[PASS] should_fail_to_parse_unfinished_lists\n");
    return 0;
}

int should_parse_a_mathematical_expression(void)
{
    fprintf(stdout, "[TEST] should_parse_a_mathematical_expression\n");

    parser_t parser;
    program_t program = {0};

    char *input = "(+ 1 2)";
    int err = parser_init(&parser, input, strlen(input));
    if (err)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: parser_init failed: %d\n", err);
        return 1;
    }

    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: parser_parse failed: %d\n", err);
        return 1;
    }

    if (program.size != 1)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: expected 1 top-level form, got %zu\n", program.size);
        return 1;
    }

    form_t *form = &program.items[0];
    if (form->type != FORM_LIST)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: expected list, got %d\n", form->type);
        return 1;
    }

    list_t *list = &form->list;
    if (list->size != 3)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: expected list with 3 elements, got %zu\n", list->size);
        return 1;
    }

    form_t *f1 = &list->items[0];
    if (f1->type != FORM_ATOM || f1->atom.type != ATOM_SYMBOL)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: expected symbol, got %d\n", f1->type);
        return 1;
    }

    if (strncmp(f1->atom.sym.chars, "+", f1->atom.sym.len) != 0)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: expected symbol '+', got %s\n", f1->atom.sym.chars);
        return 1;
    }

    form_t *f2 = &list->items[1];
    if (f2->type != FORM_ATOM || f2->atom.type != ATOM_NUMBER)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: expected number, got %d\n", f2->type);
        return 1;
    }

    if (f2->atom.num.type != NUMBER_INTEGER || f2->atom.num.integer != 1)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: expected number 1, got %lld\n", f2->atom.num.integer);
        return 1;
    }

    form_t *f3 = &list->items[2];
    if (f3->type != FORM_ATOM || f3->atom.type != ATOM_NUMBER)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: expected number, got %d\n", f3->type);
        return 1;
    }

    if (f3->atom.num.type != NUMBER_INTEGER || f3->atom.num.integer != 2)
    {
        fprintf(stderr, "[FAIL] should_parse_a_mathematical_expression: expected number 2, got %lld\n", f3->atom.num.integer);
        return 1;
    }

    parser_free_program(&program);
    fprintf(stdout, "[PASS] should_parse_a_mathematical_expression\n");
    return 0;
}

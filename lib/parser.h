#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "dynarray.h"

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    lexer_t lexer;
    token_t current_token;
} parser_t;

typedef enum
{
    NUMBER_INTEGER,
    NUMBER_FLOAT,
} number_type_t;

typedef struct
{
    number_type_t type;
    union
    {
        int64_t integer;
        double float_num;
    };
} number_t;

typedef struct
{
    size_t len;
    char *chars;
} symbol_t;

typedef struct
{
    size_t len;
    char *chars;
} string_t;

typedef enum
{
    ATOM_NUMBER,
    ATOM_SYMBOL,
    ATOM_STRING,
} atom_type_t;

typedef struct
{
    atom_type_t type;

    union
    {
        number_t num;
        symbol_t sym;
        string_t str;
    };
} atom_t;

typedef struct form form_t;
typedef DYNARRAY(form_t) list_t;

typedef enum
{
    FORM_ATOM,
    FORM_LIST,
} form_type_t;

/**
 * A form can be an atom or a list.
 */
typedef struct form
{
    form_type_t type;

    union
    {
        atom_t atom;
        list_t list;
    };
} form_t;

typedef DYNARRAY(form_t) program_t;

#define PARSER_ERR_PARSER_NOT_DEFINED -1
#define PARSER_ERR_INPUT_NOT_DEFINED -2
#define PARSER_ERR_PROGRAM_NOT_DEFINED -3
#define PARSER_ERR_LIST_NOT_DEFINED -4
#define PARSER_ERR_FORM_NOT_DEFINED -5
#define PARSER_ERR_ATOM_NOT_DEFINED -6
#define PARSER_ERR_NUMBER_NOT_DEFINED -7
#define PARSER_ERR_NUMBER_TOO_LARGE -8
#define PARSER_ERR_FAILED_TO_PARSE_NUMBER -9
#define PARSER_ERR_STRING_NOT_DEFINED -10
#define PARSER_ERR_SYMBOL_NOT_DEFINED -11
#define PARSER_ERR_EXPECTED_LPAREN -12
#define PARSER_ERR_UNEXPECTED_EOF -13

int parser_init(parser_t *parser, char *input, size_t input_len);
int parser_parse(parser_t *parser, program_t *program);

int parser_free_form(form_t *form);
int parser_free_program(program_t *program);

#ifdef PARSER_TESTS
int __program_equals(program_t *p1, program_t *p2);
int __form_equals(form_t *f1, form_t *f2);
int __atom_equals(atom_t *a1, atom_t *a2);
int __list_equals(list_t *l1, list_t *l2);
int __number_equals(number_t *n1, number_t *n2);
int __symbol_equals(symbol_t *s1, symbol_t *s2);
int __string_equals(string_t *s1, string_t *s2);
#endif

#endif

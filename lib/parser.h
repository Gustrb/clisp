#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "dynarray.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    lexer_t lexer;
    token_t current_token;
} parser_t;

typedef struct {
    enum {
        NUMBER_INTEGER,
        NUMBER_FLOAT,
    } type;
    union {
        int64_t integer;
        double float_num;
    };
} number_t;

typedef struct {
    size_t len;
    const char *chars;
} symbol_t;

typedef struct {
    size_t len;
    const char *chars;
} string_t;

typedef struct {
    enum {
        ATOM_NUMBER,
        ATOM_SYMBOL,
        ATOM_STRING,
    } type;

    union {
        number_t num;
        symbol_t sym;
        string_t str;
    };
} atom_t;


typedef struct form form_t;
typedef DYNARRAY(form_t) list_t;


/**
 * A form can be an atom or a list.
 */
typedef struct form {
    enum {
        FORM_ATOM,
        FORM_LIST,
    } type;

    union {
        atom_t atom;
        list_t list;
    };
} form_t;

typedef DYNARRAY(form_t) program_t;

#define PARSER_ERR_PARSER_NOT_DEFINED       -1
#define PARSER_ERR_INPUT_NOT_DEFINED        -2
#define PARSER_ERR_PROGRAM_NOT_DEFINED      -3
#define PARSER_ERR_LIST_NOT_DEFINED         -4
#define PARSER_ERR_FORM_NOT_DEFINED         -5
#define PARSER_ERR_ATOM_NOT_DEFINED         -6
#define PARSER_ERR_NUMBER_NOT_DEFINED       -7
#define PARSER_ERR_NUMBER_TOO_LARGE         -8
#define PARSER_ERR_FAILED_TO_PARSE_NUMBER   -9
#define PARSER_ERR_STRING_NOT_DEFINED       -10
#define PARSER_ERR_SYMBOL_NOT_DEFINED       -11

int parser_init(parser_t *parser, const char *input, size_t input_len);
int parser_parse(parser_t *parser, program_t *program);

#endif

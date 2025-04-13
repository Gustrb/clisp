#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
    lexer_t lexer;
    token_t current_token;
} parser_t;

typedef struct {
    enum {
        INTEGER,
        FLOAT,
    } type;
    union {
        int64_t integer;
        double float_num;
    };
} number_t;

typedef struct {
    size_t len;
    char *chars;
} symbol_t;

typedef struct {
    size_t len;
    char *chars;
} string_t;

typedef struct {
    enum {
        NUMBER,
        SYMBOL,
        STRING,
    } type;

    union {
        number_t num;
        symbol_t sym;
        string_t str;
    };
} atom_t;

typedef struct {
    form_t *items;
    size_t size;
    size_t capacity;
} list_t;

/**
 * A form can be an atom or a list.
 */
typedef struct {
    enum {
        FORM_ATOM,
        FORM_LIST,
    } type;

    union {
        atom_t atom;
        list_t list;
    };
} form_t;

/**
 * A program is a list of zero or more forms.
 */
typedef struct {
    size_t size;
    size_t capacity;
    form_t *items;
} program_t;

#define PARSER_ERR_PARSER_NOT_DEFINED       -1
#define PARSER_ERR_PROGRAM_NOT_DEFINED      -2
#define PARSER_ERR_MEMORY_ALLOCATION_FAILED -3

int parser_init(parser_t *parser, const char *input, size_t input_len);
int parser_parse(parser_t *parser, program_t *program);

#endif

#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    TOK_EOF,
    TOK_INTEGER,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_STRING,
    TOK_MULTIPLY,
    TOK_PLUS,
} token_type_t;

typedef struct {
    token_type_t type;

    const char *start;
    size_t len;
} token_t;

typedef struct {
    const char *input;
    size_t input_len;
    size_t pos;
} lexer_t;

#define LEXER_ERR_LEXER_NOT_DEFINED    -1
#define LEXER_ERR_INPUT_CANNOT_BE_NULL -2

int lexer_init(lexer_t *lexer, const char *input, size_t input_len);

#define LEXER_ERR_TOKEN_NOT_DEFINED    -3
#define LEXER_ERR_UNKNOWN_TOKEN        -4

int lexer_next_token(lexer_t *lexer, token_t *token);

#endif

#include <stdio.h>

#include "lexer.h"

#define IS_ALPHABETICAL(c) ((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z')
#define IS_ALPHANUMERICAL(c) IS_ALPHABETICAL(c) || IS_DIGIT(c)
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

typedef enum {
    LEXER_STATE_START,
    LEXER_STATE_INTEGER,
    LEXER_STATE_STRING,
    LEXER_STATE_STRING_LITERAL,
} lexer_state_t;

int lexer_init(lexer_t *lexer, const char *input, size_t input_len)
{
    if (!lexer) return LEXER_ERR_LEXER_NOT_DEFINED;
    if (!input) return LEXER_ERR_INPUT_CANNOT_BE_NULL;

    lexer->input = input;
    lexer->input_len = input_len;
    lexer->pos = 0;

    return 0;
}

static void __lexer_skip_whitespace(lexer_t *lexer)
{
    while (lexer->pos < lexer->input_len && 
           (lexer->input[lexer->pos] == ' ' || 
            lexer->input[lexer->pos] == '\t' || 
            lexer->input[lexer->pos] == '\n' || 
            lexer->input[lexer->pos] == '\r'))
    {
        lexer->pos++;
    }
}

int lexer_next_token(lexer_t *lexer, token_t *token)
{
    if (!lexer) return LEXER_ERR_LEXER_NOT_DEFINED;
    if (!token) return LEXER_ERR_TOKEN_NOT_DEFINED;

    __lexer_skip_whitespace(lexer);

    if (lexer->pos >= lexer->input_len)
    {
        token->type = TOK_EOF;
        token->start = NULL;
        token->len = 0;
        return 0;
    }

    lexer_state_t state = LEXER_STATE_START;
    size_t start;
    char c = lexer->input[lexer->pos];
    int err = 0;

    while (1) {
lexer_loop:
        if (lexer->pos >= lexer->input_len && state == LEXER_STATE_STRING_LITERAL)
        {
            err = LEXER_ERR_UNTERMINATED_STRING_LITERAL;
            goto falltrough;
        }

        switch (state)
        {
            case LEXER_STATE_START: {
                if (IS_DIGIT(c))
                {
                    start = lexer->pos;
                    state = LEXER_STATE_INTEGER;
                    goto lexer_loop;
                }
                if (IS_ALPHABETICAL(c))
                {
                    start = lexer->pos;
                    state = LEXER_STATE_STRING;
                    goto lexer_loop;
                }
                switch (c)
                {
                    case '(': {
                        token->type = TOK_LPAREN;
                        token->start = &lexer->input[lexer->pos];
                        token->len = 1;
                        lexer->pos++;
                        goto falltrough;
                    }
                    case ')': {
                        token->type = TOK_RPAREN;
                        token->start = &lexer->input[lexer->pos];
                        token->len = 1;
                        lexer->pos++;
                        goto falltrough;
                    }
                    case '*': {
                        token->type = TOK_MULTIPLY;
                        token->start = &lexer->input[lexer->pos];
                        token->len = 1;
                        lexer->pos++;
                        goto falltrough;
                    }
                    case '+': {
                        token->type = TOK_PLUS;
                        token->start = &lexer->input[lexer->pos];
                        token->len = 1;
                        lexer->pos++;
                        goto falltrough;
                    }
                    case '\"': {
                        start = lexer->pos;
                        state = LEXER_STATE_STRING_LITERAL;
                        lexer->pos++;
                        c = lexer->input[lexer->pos];
                        goto lexer_loop;
                    }
                    default: {
                        err = LEXER_ERR_UNKNOWN_TOKEN;
                        lexer->pos++;
                        goto falltrough;
                    }
                }
            }; break;

            case LEXER_STATE_STRING_LITERAL: {
                switch (c)
                {
                    case '\"': {
                        token->type = TOK_STRING_LITERAL;
                        token->start = &lexer->input[start];
                        token->len = lexer->pos - start + 1;
                        lexer->pos++;
                        goto falltrough;
                    };
                    default: {
                        lexer->pos++;
                        c = lexer->input[lexer->pos];
                        goto lexer_loop;
                    };
                }
            }; break;

            case LEXER_STATE_INTEGER: {
                if (IS_DIGIT(c))
                {
                    lexer->pos++;
                    c = lexer->input[lexer->pos];
                    goto lexer_loop;
                }
                else
                {
                    token->type = TOK_INTEGER;
                    token->start = &lexer->input[start];
                    token->len = lexer->pos - start;
                    goto falltrough;
                }
            }; break;

            case LEXER_STATE_STRING: {
                if (IS_ALPHANUMERICAL(c))
                {
                    lexer->pos++;
                    c = lexer->input[lexer->pos];
                    goto lexer_loop;
                }
                else
                {
                    token->type = TOK_STRING;
                    token->start = &lexer->input[start];
                    token->len = lexer->pos - start;
                    goto falltrough;
                }
            }
        }
    }

falltrough:
    return err;
}

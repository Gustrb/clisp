#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

static inline int assert_token(
    char *expected,
    size_t expected_len,
    token_type_t expected_type,
    token_t *actual
) {
    if (expected_type != actual->type)                       return 1;
    if (expected_len  != actual->len)                        return 1;
    if (strncmp(expected, actual->start, expected_len) != 0) return 1;
    return 0;
}

static inline int assert_token_type(
    token_type_t expected,
    token_type_t actual
) {
    return (expected != actual) ? 1 : 0;
}

int should_be_able_to_init_a_lexer(void);
int should_be_able_to_lex_a_string(void);
int should_be_able_to_skip_whitespace(void);
int should_be_able_to_lex_an_empty_string(void);
int should_be_able_to_lex_a_program_with_strings(void);
int should_be_able_to_lex_a_string_literal(void);
int should_be_able_to_report_an_unterminated_string_literal(void);
int should_be_able_to_lex_a_signed_integer(void);

int main(void)
{
    int err = 0;

    err = err || should_be_able_to_init_a_lexer();
    err = err || should_be_able_to_lex_an_empty_string();
    err = err || should_be_able_to_skip_whitespace();
    err = err || should_be_able_to_lex_a_string();
    err = err || should_be_able_to_lex_a_program_with_strings();
    err = err || should_be_able_to_lex_a_string_literal();
    err = err || should_be_able_to_report_an_unterminated_string_literal();
    err = err || should_be_able_to_lex_a_signed_integer();

    if (err == 0)
    {
        printf("[OK] All tests passed\n");
    }
    else
    {
        printf("[FAIL] Some tests failed\n");
    }
    return 0;
}

int should_be_able_to_init_a_lexer(void)
{
    fprintf(
        stdout,
        "[TEST] should_be_able_to_init_a_lexer\n"
    );

    int err = 0;
    
    err = lexer_init(NULL, NULL, 0);

    if (err != LEXER_ERR_LEXER_NOT_DEFINED)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_init_a_lexer: expected LEXER_ERR_LEXER_NOT_DEFINED, got %d\n",
            err
        );
        return 1;
    }

    lexer_t l;
    err = lexer_init(&l, NULL, 0);
    if (err != LEXER_ERR_INPUT_CANNOT_BE_NULL)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_init_a_lexer: expected LEXER_ERR_INPUT_CANNOT_BE_NULL, got %d\n",
            err
        );
        return 1;
    }

    const char *input = "test";
    size_t input_len = 4;
    err = lexer_init(&l, input, input_len);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_init_a_lexer: expected 0, got %d\n",
            err
        );
        return 1;
    }

    fprintf(
        stdout,
        "[PASS] should_be_able_to_init_a_lexer\n"
    );

    return 0;
}

int should_be_able_to_lex_an_empty_string(void)
{
    fprintf(
        stdout,
        "[TEST] should_be_able_to_lex_an_empty_string\n"
    );

    lexer_t l;
    const char *input = "";
    size_t input_len = 0;
    int err = lexer_init(&l, input, input_len);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_an_empty_string: expected 0, got %d\n",
            err
        );
        return 1;
    }

    token_t token;
    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_an_empty_string: expected 0, got %d\n",
            err
        );
        return 1;
    }

    err = assert_token_type(TOK_EOF, token.type);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_an_empty_string: expected token type %d, got %d\n",
            TOK_EOF,
            token.type
        );
        return 1;
    }

    fprintf(
        stdout,
        "[PASS] should_be_able_to_lex_an_empty_string\n"
    );

    return 0;
}

int should_be_able_to_skip_whitespace(void)
{
    fprintf(
        stdout,
        "[TEST] should_be_able_to_skip_whitespace\n"
    );

    lexer_t l;
    const char *input = "  \t\n\r  ";
    size_t input_len = strlen(input);
    int err = lexer_init(&l, input, input_len);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_skip_whitespace: expected 0, got %d\n",
            err
        );
        return 1;
    }

    token_t token;
    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_skip_whitespace: expected 0, got %d\n",
            err
        );
        return 1;
    }

    err = assert_token_type(TOK_EOF, token.type);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_skip_whitespace: expected token type %d, got %d\n",
            TOK_EOF,
            token.type
        );
        return 1;
    }
    fprintf(
        stdout,
        "[PASS] should_be_able_to_skip_whitespace\n"
    );
    return 0;
}

int should_be_able_to_lex_a_string(void)
{
    fprintf(
        stdout,
        "[TEST] should_be_able_to_lex_a_string\n"
    );

    lexer_t l;
    const char *input = "(* 5 (+ 2 5))";
    size_t input_len = strlen(input);

    int err = lexer_init(&l, input, input_len);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }

    token_t token;
    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }

    char *expected = "(";
    size_t expected_len = 1;
    token_type_t expected_type = TOK_LPAREN;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = "*";
    expected_len = 1;
    expected_type = TOK_MULTIPLY;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = "5";
    expected_len = 1;
    expected_type = TOK_INTEGER;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = "(";
    expected_len = 1;
    expected_type = TOK_LPAREN;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = "+";
    expected_len = 1;
    expected_type = TOK_PLUS;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }
    expected = "2";
    expected_len = 1;
    expected_type = TOK_INTEGER;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }
    expected = "5";
    expected_len = 1;
    expected_type = TOK_INTEGER;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }
    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }
    expected = ")";
    expected_len = 1;
    expected_type = TOK_RPAREN;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = ")";
    expected_len = 1;
    expected_type = TOK_RPAREN;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string: expected 0, got %d\n",
            err
        );
        return 1;
    }

    err = assert_token_type(TOK_EOF, token.type);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_skip_whitespace: expected token type %d, got %d\n",
            TOK_EOF,
            token.type
        );
        return 1;
    }

    fprintf(
        stdout,
        "[PASS] should_be_able_to_lex_a_string\n"
    );

    return 0;
}

int should_be_able_to_lex_a_program_with_strings(void)
{
    fprintf(
        stdout,
        "[TEST] should_be_able_to_lex_a_program_with_strings\n"
    );

    lexer_t l;
    const char *input = "(dog (cat))";
    size_t input_len = strlen(input);
    int err = lexer_init(&l, input, input_len);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected 0, got %d\n",
            err
        );
        return 1;
    }

    token_t token;
    err = lexer_next_token(&l, &token);

    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected 0, got %d\n",
            err
        );
        return 1;
    }

    char *expected = "(";
    size_t expected_len = 1;
    token_type_t expected_type = TOK_LPAREN;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = "dog";
    expected_len = 3;
    expected_type = TOK_STRING;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = "(";
    expected_len = 1;
    expected_type = TOK_LPAREN;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = "cat";
    expected_len = 3;
    expected_type = TOK_STRING;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = ")";
    expected_len = 1;
    expected_type = TOK_RPAREN;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = ")";
    expected_len = 1;
    expected_type = TOK_RPAREN;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected 0, got %d\n",
            err
        );
        return 1;
    }

    err = assert_token_type(TOK_EOF, token.type);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_program_with_strings: expected token type %d, got %d\n",
            TOK_EOF,
            token.type
        );
        return 1;
    }

    fprintf(
        stdout,
        "[PASS] should_be_able_to_lex_a_program_with_strings\n"
    );
    
    return 0;
}

int should_be_able_to_lex_a_string_literal(void)
{
    fprintf(
        stderr,
        "[TEST] should_be_able_to_lex_a_string_literal\n"
    );

    lexer_t l;
    const char *input = "\"hello world\"";
    size_t input_len = strlen(input);

    int err = lexer_init(&l, input, input_len);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string_literal: expected 0, got %d\n",
            err
        );
        return 1;
    }

    token_t token;
    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string_literal: expected 0, got %d\n",
            err
        );
        return 1;
    }
    char *expected = "\"hello world\"";
    size_t expected_len = strlen(expected);
    token_type_t expected_type = TOK_STRING_LITERAL;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string_literal: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }
    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string_literal: expected 0, got %d\n",
            err
        );
        return 1;
    }
    err = assert_token_type(TOK_EOF, token.type);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_string_literal: expected token type %d, got %d\n",
            TOK_EOF,
            token.type
        );
        return 1;
    }

    fprintf(
        stdout,
        "[PASS] should_be_able_to_lex_a_string_literal\n"
    );

    return 0;
}

int should_be_able_to_report_an_unterminated_string_literal(void)
{
    fprintf(
        stdout,
        "[TEST] should_be_able_to_report_an_unterminated_string_literal\n"
    );

    lexer_t l;
    const char *input = "\"hello world";
    size_t input_len = strlen(input);
    int err = lexer_init(&l, input, input_len);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_report_an_unterminated_string_literal: expected 0, got %d\n",
            err
        );
        return 1;
    }

    token_t token;
    err = lexer_next_token(&l, &token);

    if (err != LEXER_ERR_UNTERMINATED_STRING_LITERAL)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_report_an_unterminated_string_literal: expected LEXER_ERR_UNTERMINATED_STRING_LITERAL, got %d\n",
            err
        );
        return 1;
    }

    err = assert_token_type(TOK_EOF, token.type);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_report_an_unterminated_string_literal: expected token type %d, got %d\n",
            TOK_EOF,
            token.type
        );
        return 1;
    }

    fprintf(
        stdout,
        "[PASS] should_be_able_to_report_an_unterminated_string_literal\n"
    );

    return 0;
}

int should_be_able_to_lex_a_signed_integer(void)
{
    fprintf(
        stdout,
        "[TEST] should_be_able_to_lex_a_signed_integer\n"
    );

    lexer_t l;
    const char *input = "-123";
    size_t input_len = strlen(input);
    int err = lexer_init(&l, input, input_len);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_signed_integer: expected 0, got %d\n",
            err
        );
        return 1;
    }

    token_t token;
    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_signed_integer: expected 0, got %d\n",
            err
        );
        return 1;
    }

    char *expected = "-123";
    size_t expected_len = strlen(expected);
    token_type_t expected_type = TOK_INTEGER;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_signed_integer: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_signed_integer: expected 0, got %d\n",
            err
        );
        return 1;
    }
    err = assert_token_type(TOK_EOF, token.type);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_signed_integer: expected token type %d, got %d\n",
            TOK_EOF,
            token.type
        );
        return 1;
    }

    input = "+123";
    input_len = strlen(input);
    err = lexer_init(&l, input, input_len);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_signed_integer: expected 0, got %d\n",
            err
        );
        return 1;
    }

    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_signed_integer: expected 0, got %d\n",
            err
        );
        return 1;
    }

    expected = "+123";
    expected_len = strlen(expected);
    expected_type = TOK_INTEGER;
    err = assert_token(expected, expected_len, expected_type, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_signed_integer: expected token %.*s, got %.*s\n",
            (int)expected_len,
            expected,
            (int)token.len,
            token.start
        );
        return 1;
    }
    err = lexer_next_token(&l, &token);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_signed_integer: expected 0, got %d\n",
            err
        );
        return 1;
    }
    err = assert_token_type(TOK_EOF, token.type);
    if (err != 0)
    {
        fprintf(
            stderr,
            "[FAIL] should_be_able_to_lex_a_signed_integer: expected token type %d, got %d\n",
            TOK_EOF,
            token.type
        );
        return 1;
    }

    fprintf(
        stdout,
        "[PASS] should_be_able_to_lex_a_signed_integer\n"
    );

    return 0;
}

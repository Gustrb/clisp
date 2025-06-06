#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "parser.h"

int parser_init(parser_t *parser, char *input, size_t input_len)
{
    if (!parser)
        return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!input)
        return PARSER_ERR_INPUT_NOT_DEFINED;

    int err = lexer_init(&parser->lexer, input, input_len);
    if (err)
        return err;

    err = lexer_next_token(&parser->lexer, &parser->current_token);
    if (err)
        return err;

    return 0;
}

int parser_parse_form(parser_t *parser, form_t *form);
int parser_parse_list(parser_t *parser, list_t *list);
int parser_parse_atom(parser_t *parser, atom_t *atom);

int parser_parse_number(parser_t *parser, number_t *number);
int parser_parse_integer(parser_t *parser, int64_t *val);
int parser_parse_float(parser_t *parser, double *val);

int parser_parse_string(parser_t *parser, string_t *string);
int parser_parse_symbol(parser_t *parser, symbol_t *symbol);

int parser_parse(parser_t *parser, program_t *program)
{
    if (!parser)
        return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!program)
        return PARSER_ERR_PROGRAM_NOT_DEFINED;

    int err;
    while (parser->current_token.type != TOK_EOF)
    {
        form_t form;
        err = parser_parse_form(parser, &form);
        if (err)
            return err;

        DYNARRAY_PUSH(*program, form, form_t);

        err = lexer_next_token(&parser->lexer, &parser->current_token);
        if (err)
            return err;
    }

    return 0;
}

int parser_parse_form(parser_t *parser, form_t *form)
{
    if (!parser)
        return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!form)
        return PARSER_ERR_FORM_NOT_DEFINED;

    int err;
    if (parser->current_token.type == TOK_LPAREN)
    {
        list_t list;
        form->type = FORM_LIST;
        list.items = NULL;
        list.size = 0;
        list.capacity = 0;
        err = parser_parse_list(parser, &list);
        if (err)
            return err;

        form->list = list;
    }
    else
    {
        atom_t atom;
        form->type = FORM_ATOM;
        err = parser_parse_atom(parser, &atom);
        if (err)
            return err;

        form->atom = atom;
    }

    return 0;
}

int parser_parse_list(parser_t *parser, list_t *list)
{
    if (!parser)
        return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!list)
        return PARSER_ERR_LIST_NOT_DEFINED;
    if (parser->current_token.type != TOK_LPAREN)
        return PARSER_ERR_EXPECTED_LPAREN;

    int err = lexer_next_token(&parser->lexer, &parser->current_token);
    if (err)
        return err;

    while (parser->current_token.type != TOK_RPAREN)
    {
        if (parser->current_token.type == TOK_EOF)
        {
            return PARSER_ERR_UNEXPECTED_EOF;
        }

        form_t form;
        err = parser_parse_form(parser, &form);
        if (err)
            return err;

        DYNARRAY_PUSH(*list, form, form_t);

        err = lexer_next_token(&parser->lexer, &parser->current_token);
        if (err)
            return err;
    }

    return 0;
}

int parser_parse_atom(parser_t *parser, atom_t *atom)
{
    if (!parser)
        return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!atom)
        return PARSER_ERR_ATOM_NOT_DEFINED;

    int err;
    // Atom can be a number, a symbol, or a string
    if (parser->current_token.type == TOK_INTEGER || parser->current_token.type == TOK_FLOAT)
    {
        atom->type = ATOM_NUMBER;
        err = parser_parse_number(parser, &atom->num);
        if (err)
            return err;
    }
    else if (parser->current_token.type == TOK_STRING_LITERAL)
    {
        atom->type = ATOM_STRING;
        err = parser_parse_string(parser, &atom->str);
        if (err)
            return err;
    }
    else if (parser->current_token.type == TOK_STRING)
    {
        atom->type = ATOM_SYMBOL;
        err = parser_parse_symbol(parser, &atom->sym);
        if (err)
            return err;
    }
    else if (parser->current_token.type == TOK_PLUS || parser->current_token.type == TOK_MINUS || parser->current_token.type == TOK_MULTIPLY)
    {
        atom->type = ATOM_SYMBOL;
        err = parser_parse_symbol(parser, &atom->sym);
        if (err)
            return err;
    }

    return 0;
}

int parser_parse_number(parser_t *parser, number_t *number)
{
    if (!parser)
        return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!number)
        return PARSER_ERR_NUMBER_NOT_DEFINED;

    assert(parser->current_token.type == TOK_INTEGER || parser->current_token.type == TOK_FLOAT);

    int err;
    if (parser->current_token.type == TOK_INTEGER)
    {
        number->type = NUMBER_INTEGER;
        err = parser_parse_integer(parser, &number->integer);
        if (err)
            return err;
    }
    else
    {
        number->type = NUMBER_FLOAT;
        err = parser_parse_float(parser, &number->float_num);
        if (err)
            return err;
    }

    return 0;
}

int parser_parse_integer(parser_t *parser, int64_t *val)
{
    if (!parser)
        return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!val)
        return PARSER_ERR_NUMBER_NOT_DEFINED;

    if (parser->current_token.len > 31)
    {
        return PARSER_ERR_NUMBER_TOO_LARGE;
    }

    char buff[32];
    for (size_t i = 0; i < parser->current_token.len; ++i)
    {
        buff[i] = parser->current_token.start[i];
    }

    buff[parser->current_token.len] = '\0';
    char *endptr = NULL;
    *val = strtoll(buff, &endptr, 10);

    if (endptr == buff || *endptr != '\0')
    {
        return PARSER_ERR_FAILED_TO_PARSE_NUMBER;
    }

    return 0;
}

int parser_parse_float(parser_t *parser, double *val)
{
    if (!parser)
        return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!val)
        return PARSER_ERR_NUMBER_NOT_DEFINED;

    char buff[32];
    if (parser->current_token.len > 31)
    {
        return PARSER_ERR_NUMBER_TOO_LARGE;
    }

    for (size_t i = 0; i < parser->current_token.len; ++i)
    {
        buff[i] = parser->current_token.start[i];
    }
    buff[parser->current_token.len] = '\0';

    char *endptr = NULL;
    *val = strtod(buff, &endptr);

    if (endptr == buff || *endptr != '\0')
    {
        return PARSER_ERR_FAILED_TO_PARSE_NUMBER;
    }

    return 0;
}

int parser_parse_string(parser_t *parser, string_t *string)
{
    if (!parser)
        return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!string)
        return PARSER_ERR_STRING_NOT_DEFINED;

    string->chars = parser->current_token.start;
    string->len = parser->current_token.len;

    return 0;
}

int parser_parse_symbol(parser_t *parser, symbol_t *symbol)
{
    if (!parser)
        return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!symbol)
        return PARSER_ERR_SYMBOL_NOT_DEFINED;

    symbol->chars = parser->current_token.start;
    symbol->len = parser->current_token.len;

    return 0;
}

int parser_free_program(program_t *program)
{
    if (!program)
        return PARSER_ERR_PROGRAM_NOT_DEFINED;

    for (size_t i = 0; i < program->size; ++i)
    {
        form_t *form = &program->items[i];
        int err = parser_free_form(form);
        if (err)
            return err;
    }

    free(program->items);
    program->items = NULL;
    program->size = 0;
    program->capacity = 0;

    return 0;
}

int parser_free_form(form_t *form)
{
    if (!form)
        return PARSER_ERR_FORM_NOT_DEFINED;

    if (form->type == FORM_LIST)
    {
        for (size_t i = 0; i < form->list.size; ++i)
        {
            form_t *subform = &form->list.items[i];
            int err = parser_free_form(subform);
            if (err)
                return err;
        }
        free(form->list.items);
        form->list.items = NULL;
        form->list.size = 0;
        form->list.capacity = 0;
    }

    return 0;
}

#ifdef PARSER_TESTS

int __program_equals(program_t *p1, program_t *p2)
{
    if (!p1 || !p2)
        return 0;

    if (p1 == p2)
        return 1;

    if (p1->size != p2->size)
        return 0;

    for (size_t i = 0; i < p1->size; ++i)
    {
        form_t *f1 = &p1->items[i];
        form_t *f2 = &p2->items[i];

        if (!__form_equals(f1, f2))
            return 0;
    }

    return 1;
}

int __form_equals(form_t *f1, form_t *f2)
{
    if (!f1 || !f2)
        return 0;

    if (f1 == f2)
        return 1;

    if (f1->type != f2->type)
        return 0;

    switch (f1->type)
    {
    case FORM_ATOM:
    {
        atom_t *a1 = &f1->atom;
        atom_t *a2 = &f2->atom;

        return __atom_equals(a1, a2);
    }
    case FORM_LIST:
    {
        list_t *l1 = &f1->list;
        list_t *l2 = &f2->list;

        return __list_equals(l1, l2);
    }
    default:
    {
        return 0;
    }
    }

    return 0;
}

int __atom_equals(atom_t *a1, atom_t *a2)
{
    if (!a1 || !a2)
        return 0;

    if (a1 == a2)
        return 1;

    if (a1->type != a2->type)
        return 0;

    switch (a1->type)
    {
    case ATOM_NUMBER:
    {
        number_t *n1 = &a1->num;
        number_t *n2 = &a2->num;

        if (n1->type != n2->type)
            return 0;

        return __number_equals(n1, n2);
    }
    case ATOM_SYMBOL:
    {
        symbol_t *s1 = &a1->sym;
        symbol_t *s2 = &a2->sym;

        return __symbol_equals(s1, s2);
    }
    case ATOM_STRING:
    {
        string_t *s1 = &a1->str;
        string_t *s2 = &a2->str;

        return __string_equals(s1, s2);
    }
    default:
    {
        return 0;
    }
    }

    return 0;
}

int __list_equals(list_t *l1, list_t *l2)
{
    if (!l1 || !l2)
        return 0;

    if (l1 == l2)
        return 1;

    if (l1->size != l2->size)
        return 0;

    for (size_t i = 0; i < l1->size; ++i)
    {
        form_t *f1 = &l1->items[i];
        form_t *f2 = &l2->items[i];

        if (!__form_equals(f1, f2))
            return 0;
    }

    return 1;
}

int __number_equals(number_t *n1, number_t *n2)
{
    if (!n1 || !n2)
        return 0;

    if (n1 == n2)
        return 1;

    if (n1->type != n2->type)
        return 0;

    switch (n1->type)
    {
    case NUMBER_INTEGER:
    {
        return n1->integer == n2->integer;
    }
    case NUMBER_FLOAT:
    {
        return n1->float_num == n2->float_num;
    }
    default:
    {
        return 0;
    }
    }

    return 0;
}

int __symbol_equals(symbol_t *s1, symbol_t *s2)
{
    if (!s1 || !s2)
        return 0;

    if (s1 == s2)
        return 1;

    if (s1->len != s2->len)
        return 0;

    return strncmp(s1->chars, s2->chars, s1->len) == 0;
}

int __string_equals(string_t *s1, string_t *s2)
{
    if (!s1 || !s2)
        return 0;

    if (s1 == s2)
        return 1;

    if (s1->len != s2->len)
        return 0;

    return strncmp(s1->chars, s2->chars, s1->len) == 0;
}

#endif

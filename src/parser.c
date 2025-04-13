#include <string.h>

#include "parser.h"

#define dyarray_push_back(array, item) \
    do {         \
        if (array->size >= array->capacity) { \
            size_t new_capacity = array->capacity * 2; \
            if (new_capacity < 4) new_capacity = 4; \
            array->items = realloc(array->items, new_capacity * sizeof(*array->items)); \
            if (!array->items) return PARSER_ERR_MEMORY_ALLOCATION_FAILED; \
            array->capacity = new_capacity; \
        } \
        array->items[array->size++] = item; \
    } while (0); \

int parser_init(parser_t *parser, const char *input, size_t input_len)
{
    if (parser == NULL) {
        return PARSER_ERR_PARSER_NOT_DEFINED;
    }

    int err = lexer_init(&parser->lexer, input, input_len);
    if (err != 0) return err;

    return 0;
}

int __parser_parse_form(parser_t *parser, form_t *form);
int __parser_parse_list(parser_t *parser, list_t *list);
int __parser_parse_atom(parser_t *parser, atom_t *atom);

int parser_parse(parser_t *parser, program_t *program)
{
    if (!parser) return PARSER_ERR_PARSER_NOT_DEFINED;
    if (!program) return PARSER_ERR_PROGRAM_NOT_DEFINED;

    int err;
    while (1)
    {
        err = lexer_next_token(&parser->lexer, &parser->current_token);
        if (err != 0) return err;
        if (parser->current_token.type == TOK_EOF) break;

        form_t form;
        err = __parser_parse_form(parser, &form);
        if (err != 0) return err;

        dyarray_push_back(program, form);
        if (err != 0) return err;
    }
}

int __parser_parse_form(parser_t *parser, form_t *form)
{
    memset(form, 0, sizeof(form_t));
    if (parser->current_token.type == TOK_LPAREN)
    {
        list_t list = {0};
        form->type = FORM_LIST;

        int err = __parser_parse_list(parser, &list);
        if (err != 0) return err;

        form->list = list;
        return 0;
    }

    form->type = FORM_ATOM;
    atom_t atom = {0};
    int err = __parser_parse_atom(parser, &atom);
    if (err != 0) return err;
    form->atom = atom;
    return 0;
}

int __parser_parse_list(parser_t *parser, list_t *list)
{

}

int __parser_parse_atom(parser_t *parser, atom_t *atom)
{

}

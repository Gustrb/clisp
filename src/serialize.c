#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "serialize.h"

#define BIG_ENDIAN_WRITE(buffer, value, type)      \
    do                                             \
    {                                              \
        union                                      \
        {                                          \
            type v;                                \
            char c[sizeof(type)];                  \
        } u = {.v = value};                        \
        for (size_t i = 0; i < sizeof(type); ++i)  \
        {                                          \
            buffer[i] = u.c[sizeof(type) - i - 1]; \
        }                                          \
    } while (0)

#define BIG_ENDIAN_READ(buffer, value, type)       \
    do                                             \
    {                                              \
        union                                      \
        {                                          \
            type v;                                \
            char c[sizeof(type)];                  \
        } u;                                       \
        for (size_t i = 0; i < sizeof(type); ++i)  \
        {                                          \
            u.c[sizeof(type) - i - 1] = buffer[i]; \
        }                                          \
        value = u.v;                               \
    } while (0)

// TODO: Validate program when deserializing

int serializer_init(serializer_t *serializer, int fd)
{
    if (!serializer)
        return SERIALIZER_ERR_INVALID_ARGUMENT;
    if (fd < 0)
        return SERIALIZER_ERR_INVALID_ARGUMENT;

    serializer->fd = fd;

    return 0;
}

size_t __compute_atom_size(atom_t *atom);
size_t __compute_list_size(list_t *list);
size_t __compute_number_size(number_t *number);
size_t __compute_string_size(string_t *string);
size_t __compute_symbol_size(symbol_t *symbol);
size_t __compute_form_size(form_t *form);

size_t __encode_form(form_t *form, char *buffer);
size_t __encode_atom(atom_t *atom, char *buffer);
size_t __encode_list(list_t *list, char *buffer);
size_t __encode_number(number_t *number, char *buffer);
size_t __encode_string(string_t *string, char *buffer);
size_t __encode_symbol(symbol_t *symbol, char *buffer);

int serializer_serialize(serializer_t *serializer, program_t *program)
{
    if (!serializer)
        return SERIALIZER_ERR_INVALID_ARGUMENT;
    if (!program)
        return SERIALIZER_ERR_INVALID_ARGUMENT;

    // First thing we are going to encode is the size
    size_t numbytes = sizeof(program->size);
    for (size_t i = 0; i < program->size; ++i)
    {
        form_t *form = &program->items[i];
        numbytes += __compute_form_size(form);
    }

    // Write the total size first in big endian
    char sizebuf[sizeof(size_t)];
    BIG_ENDIAN_WRITE(sizebuf, numbytes, size_t);
    ssize_t bytes_written = write(serializer->fd, sizebuf, sizeof(size_t));
    if (bytes_written == -1)
    {
        return SERIALIZER_ERR_WRITE_FAILED;
    }

    if ((size_t)bytes_written != sizeof(size_t))
    {
        return SERIALIZER_ERR_WRITE_FAILED;
    }

    char *buffer = malloc(numbytes);
    if (!buffer)
        return SERIALIZER_ERR_MEMORY_ALLOCATION_FAILED;

    // then we need to encode the data
    char *current = buffer;

    // Write the program size
    BIG_ENDIAN_WRITE(current, program->size, size_t);
    current += sizeof(program->size);

    // Write all the forms
    for (size_t i = 0; i < program->size; ++i)
    {
        form_t *form = &program->items[i];
        size_t form_bytes = __encode_form(form, current);
        current += form_bytes;
    }

    // then write the data
    bytes_written = write(serializer->fd, buffer, numbytes);
    if (bytes_written == -1)
    {
        free(buffer);
        return SERIALIZER_ERR_WRITE_FAILED;
    }

    if ((size_t)bytes_written != numbytes)
    {
        free(buffer);
        return SERIALIZER_ERR_WRITE_FAILED;
    }

    free(buffer);

    return 0;
}

size_t __compute_atom_size(atom_t *atom)
{
    size_t numbytes = 0;

    numbytes += sizeof(atom->type);

    switch (atom->type)
    {
    case ATOM_NUMBER:
    {
        numbytes += __compute_number_size(&atom->num);
        break;
    }
    case ATOM_STRING:
    {
        numbytes += __compute_string_size(&atom->str);
        break;
    }
    case ATOM_SYMBOL:
    {
        numbytes += __compute_symbol_size(&atom->sym);
        break;
    }
    }

    return numbytes;
}

size_t __compute_list_size(list_t *list)
{
    size_t numbytes = 0;

    numbytes += sizeof(list->size);

    for (size_t i = 0; i < list->size; ++i)
    {
        form_t *form = &list->items[i];
        numbytes += __compute_form_size(form);
    }

    return numbytes;
}

size_t __compute_number_size(number_t *number)
{
    size_t numbytes = 0;

    numbytes += sizeof(number->type);

    switch (number->type)
    {
    case NUMBER_INTEGER:
    {
        numbytes += sizeof(number->integer);
        break;
    }
    case NUMBER_FLOAT:
    {
        numbytes += sizeof(number->float_num);
        break;
    }
    }

    return numbytes;
}

size_t __compute_string_size(string_t *string)
{
    size_t numbytes = 0;

    numbytes += sizeof(string->len);
    numbytes += string->len;

    return numbytes;
}

size_t __compute_symbol_size(symbol_t *symbol)
{
    size_t numbytes = 0;

    numbytes += sizeof(symbol->len);
    numbytes += symbol->len;

    return numbytes;
}

size_t __compute_form_size(form_t *form)
{
    size_t numbytes = 0;

    // Then for each form we need to encode the type
    numbytes += sizeof(form->type);

    switch (form->type)
    {
    case FORM_ATOM:
    {
        atom_t *atom = &form->atom;
        numbytes += __compute_atom_size(atom);
        break;
    }
    case FORM_LIST:
    {
        list_t *list = &form->list;
        numbytes += __compute_list_size(list);
        break;
    }
    }

    return numbytes;
}

size_t __encode_form(form_t *form, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(form->type);
    BIG_ENDIAN_WRITE(buffer, form->type, form_type_t);
    buffer += sizeof(form->type);

    switch (form->type)
    {
    case FORM_ATOM:
    {
        atom_t *atom = &form->atom;
        numbytes += __encode_atom(atom, buffer);
        break;
    }
    case FORM_LIST:
    {
        list_t *list = &form->list;
        numbytes += __encode_list(list, buffer);
        break;
    }
    }

    return numbytes;
}

size_t __encode_atom(atom_t *atom, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(atom->type);
    BIG_ENDIAN_WRITE(buffer, atom->type, atom_type_t);
    buffer += sizeof(atom->type);

    switch (atom->type)
    {
    case ATOM_NUMBER:
    {
        numbytes += __encode_number(&atom->num, buffer);
        break;
    }
    case ATOM_STRING:
    {
        numbytes += __encode_string(&atom->str, buffer);
        break;
    }
    case ATOM_SYMBOL:
    {
        numbytes += __encode_symbol(&atom->sym, buffer);
        break;
    }
    }

    return numbytes;
}

size_t __encode_list(list_t *list, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(list->size);
    BIG_ENDIAN_WRITE(buffer, list->size, size_t);
    buffer += sizeof(list->size);

    for (size_t i = 0; i < list->size; ++i)
    {
        form_t *form = &list->items[i];
        size_t form_bytes = __encode_form(form, buffer);
        numbytes += form_bytes;
        buffer += form_bytes;
    }

    return numbytes;
}

size_t __encode_number(number_t *number, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(number->type);
    BIG_ENDIAN_WRITE(buffer, number->type, number_type_t);
    buffer += sizeof(number->type);

    switch (number->type)
    {
    case NUMBER_INTEGER:
    {
        numbytes += sizeof(number->integer);
        BIG_ENDIAN_WRITE(buffer, number->integer, int64_t);
        buffer += sizeof(number->integer);
        break;
    }
    case NUMBER_FLOAT:
    {
        numbytes += sizeof(number->float_num);
        BIG_ENDIAN_WRITE(buffer, number->float_num, double);
        buffer += sizeof(number->float_num);
        break;
    }
    }

    return numbytes;
}

size_t __encode_string(string_t *string, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(string->len);
    BIG_ENDIAN_WRITE(buffer, string->len, size_t);
    buffer += sizeof(string->len);

    for (size_t i = 0; i < string->len; ++i)
    {
        buffer[i] = string->chars[i];
    }

    numbytes += string->len;

    return numbytes;
}

size_t __encode_symbol(symbol_t *symbol, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(symbol->len);
    BIG_ENDIAN_WRITE(buffer, symbol->len, size_t);
    buffer += sizeof(symbol->len);

    for (size_t i = 0; i < symbol->len; ++i)
    {
        buffer[i] = symbol->chars[i];
    }

    numbytes += symbol->len;

    return numbytes;
}

// Deserializer
size_t __deserialize_atom(atom_t *atom, char *buffer);
size_t __deserialize_form(form_t *form, char *buffer);
size_t __deserialize_list(list_t *list, char *buffer);
size_t __deserialize_number(number_t *number, char *buffer);
size_t __deserialize_string(string_t *string, char *buffer);
size_t __deserialize_symbol(symbol_t *symbol, char *buffer);

int deserializer_init(deserializer_t *deserializer, int fd)
{
    if (!deserializer)
        return SERIALIZER_ERR_INVALID_ARGUMENT;

    if (fd < 0)
        return SERIALIZER_ERR_INVALID_ARGUMENT;

    deserializer->fd = fd;

    return 0;
}

int deserializer_deserialize(deserializer_t *deserializer, program_t *program)
{
    if (!deserializer)
        return SERIALIZER_ERR_INVALID_ARGUMENT;
    if (!program)
        return SERIALIZER_ERR_INVALID_ARGUMENT;

    char sizebuf[sizeof(size_t)] = {0};
    ssize_t bytes_read = read(deserializer->fd, sizebuf, sizeof(size_t));
    if (bytes_read == -1)
    {
        return SERIALIZER_ERR_READ_FAILED;
    }

    size_t total_size = 0;
    BIG_ENDIAN_READ(sizebuf, total_size, size_t);

    char *buffer = malloc(total_size);
    if (!buffer)
        return SERIALIZER_ERR_MEMORY_ALLOCATION_FAILED;

    bytes_read = read(deserializer->fd, buffer, total_size);
    if (bytes_read == -1)
    {
        return SERIALIZER_ERR_READ_FAILED;
    }

    if ((size_t)bytes_read != total_size)
    {
        return SERIALIZER_ERR_READ_FAILED;
    }

    char *current = buffer;

    // Read the program size
    BIG_ENDIAN_READ(current, program->size, size_t);
    current += sizeof(program->size);

    program->items = malloc(program->size * sizeof(form_t));
    program->capacity = program->size;
    if (!program->items)
        return SERIALIZER_ERR_MEMORY_ALLOCATION_FAILED;

    for (size_t i = 0; i < program->size; ++i)
    {
        form_t *form = &program->items[i];
        size_t form_bytes = __deserialize_form(form, current);
        current += form_bytes;
    }

    free(buffer);

    return 0;
}

size_t __deserialize_form(form_t *form, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(form->type);
    BIG_ENDIAN_READ(buffer, form->type, form_type_t);
    buffer += sizeof(form->type);

    switch (form->type)
    {
    case FORM_ATOM:
    {
        atom_t *atom = &form->atom;
        numbytes += __deserialize_atom(atom, buffer);
        break;
    }
    case FORM_LIST:
    {
        list_t *list = &form->list;
        numbytes += __deserialize_list(list, buffer);
        break;
    }
    }

    return numbytes;
}

size_t __deserialize_atom(atom_t *atom, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(atom->type);
    BIG_ENDIAN_READ(buffer, atom->type, atom_type_t);
    buffer += sizeof(atom->type);

    switch (atom->type)
    {
    case ATOM_NUMBER:
    {
        numbytes += __deserialize_number(&atom->num, buffer);
        break;
    }
    case ATOM_STRING:
    {
        numbytes += __deserialize_string(&atom->str, buffer);
        break;
    }
    case ATOM_SYMBOL:
    {
        numbytes += __deserialize_symbol(&atom->sym, buffer);
        break;
    }
    }
    return numbytes;
}

size_t __deserialize_list(list_t *list, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(list->size);
    BIG_ENDIAN_READ(buffer, list->size, size_t);
    buffer += sizeof(list->size);

    list->items = malloc(list->size * sizeof(form_t));
    list->capacity = list->size;
    if (!list->items)
        return SERIALIZER_ERR_MEMORY_ALLOCATION_FAILED;

    for (size_t i = 0; i < list->size; ++i)
    {
        form_t *form = &list->items[i];
        size_t form_bytes = __deserialize_form(form, buffer);
        numbytes += form_bytes;
        buffer += form_bytes;
    }

    return numbytes;
}

size_t __deserialize_number(number_t *number, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(number->type);
    BIG_ENDIAN_READ(buffer, number->type, number_type_t);
    buffer += sizeof(number->type);

    switch (number->type)
    {
    case NUMBER_INTEGER:
    {
        numbytes += sizeof(number->integer);
        BIG_ENDIAN_READ(buffer, number->integer, int64_t);
        break;
    }
    case NUMBER_FLOAT:
    {
        numbytes += sizeof(number->float_num);
        BIG_ENDIAN_READ(buffer, number->float_num, double);
        break;
    }
    }

    return numbytes;
}

size_t __deserialize_string(string_t *string, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(string->len);
    BIG_ENDIAN_READ(buffer, string->len, size_t);
    buffer += sizeof(string->len);

    string->chars = malloc(string->len);
    if (!string->chars)
        return SERIALIZER_ERR_MEMORY_ALLOCATION_FAILED;

    for (size_t i = 0; i < string->len; ++i)
    {
        string->chars[i] = buffer[i];
    }

    numbytes += string->len;

    return numbytes;
}

size_t __deserialize_symbol(symbol_t *symbol, char *buffer)
{
    size_t numbytes = 0;

    numbytes += sizeof(symbol->len);
    BIG_ENDIAN_READ(buffer, symbol->len, size_t);
    buffer += sizeof(symbol->len);

    symbol->chars = malloc(symbol->len);
    if (!symbol->chars)
        return SERIALIZER_ERR_MEMORY_ALLOCATION_FAILED;

    for (size_t i = 0; i < symbol->len; ++i)
    {
        symbol->chars[i] = buffer[i];
    }

    numbytes += symbol->len;

    return numbytes;
}

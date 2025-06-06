#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <stddef.h>
#include <stdint.h>

#include "parser.h"

typedef struct
{
    int fd;
    // TODO: buffered writing
} serializer_t;

typedef struct
{
    int fd;
} deserializer_t;

#define SERIALIZER_ERR_INVALID_ARGUMENT -1
#define SERIALIZER_ERR_MEMORY_ALLOCATION_FAILED -2
#define SERIALIZER_ERR_WRITE_FAILED -3

int serializer_init(serializer_t *serializer, int fd);
int serializer_serialize(serializer_t *serializer, program_t *program);

int deserializer_init(deserializer_t *deserializer, int fd);

#define SERIALIZER_ERR_READ_FAILED -4

int deserializer_deserialize(deserializer_t *deserializer, program_t *program);

#endif

#ifndef IO_H
#define IO_H

#include <stdlib.h>

typedef struct
{
    char *data;
    size_t size;
} io_str_t;

#define ERR_NO_FILE -1
#define ERR_NO_STRING -2
#define ERR_FAILED_TO_OPEN_FILE -3
#define ERR_FAILED_TO_SEEK_FILE -4
#define ERR_FAILED_TO_FTELL_FILE -5
#define ERR_OUT_OF_MEMORY -6
#define ERR_FAILED_TO_READ_FILE -7

int io_load_file_into_memory(char *filepath, io_str_t *string);
int io_free_string(io_str_t *string);

#endif

#include <stdio.h>
#include <stdlib.h>

#include "io.h"

int io_load_file_into_memory(const char *filepath, io_str_t *string)
{
    if (!filepath) return ERR_NO_FILE;
    if (!string) return ERR_NO_STRING;

    FILE *file = fopen(filepath, "r");
    if (!file) return ERR_FAILED_TO_OPEN_FILE;

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return ERR_FAILED_TO_SEEK_FILE;
    }

    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return ERR_FAILED_TO_FTELL_FILE;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return ERR_FAILED_TO_SEEK_FILE;
    }

    string->data = (char *)malloc(size + 1);
    if (!string->data) {
        fclose(file);
        return ERR_OUT_OF_MEMORY;
    }

    ssize_t read = fread(string->data, 1, size, file);
    if (read != size) {
        free(string->data);
        fclose(file);
        return ERR_FAILED_TO_READ_FILE;
    }

    string->data[size] = '\0'; // Null-terminate the string
    string->size = size;
    fclose(file);

    return 0;
}

int io_free_string(io_str_t *string)
{
    if (!string) return ERR_NO_STRING;

    if (string->data) free(string->data);
    string->data = NULL;
    string->size = 0;

    return 0;
}

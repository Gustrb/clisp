#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "dynarray.h"
#include "io.h"
#include "benchmark.h"

typedef struct
{
    char *filename;
    program_t program;
} m_unit_t;

typedef DYNARRAY(m_unit_t) module_t;

int module_parse_files(module_t *mod, char **filenames, size_t count);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file1> <file2> ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[INFO]: Running Single-threaded parser\n");
    printf("[INFO]: Number of files: %d\n", argc);

    double start_time = benchmark_get_time();

    argv++;
    argc--;

    module_t module = {0};
    int err = module_parse_files(&module, argv, argc);
    if (err != 0)
    {
        fprintf(stderr, "Error parsing files: %d\n", err);
        return err;
    }

    for (size_t i = 0; i < module.size; ++i)
    {
        parser_free_program(&module.items[i].program);
    }

    double end_time = benchmark_get_time();
    double duration = end_time - start_time;

    fprintf(stderr, "[INFO]: Parsed %zu files successfully.\n", module.size);
    printf("[INFO]: Parsing time: %f seconds\n", duration);

    DYNARRAY_FREE(module);

    return EXIT_SUCCESS;
}

int module_parse_files(module_t *mod, char **filenames, size_t count)
{
    parser_t parser = {0};
    int err = 0;
    for (size_t i = 0; i < count; ++i)
    {
        m_unit_t unit = {0};
        // Note: the string with the file content must outlive the program
        // since the program contains pointers to the string data.
        io_str_t string = {0};
        unit.filename = filenames[i];
        err = io_load_file_into_memory(filenames[i], &string);
        if (err != 0)
        {
            fprintf(stderr, "Error loading file %s: %d\n", filenames[i], err);
            return err;
        }

        err = parser_init(&parser, string.data, string.size);
        if (err != 0)
        {
            fprintf(stderr, "Error initializing parser for file %s: %d\n", filenames[i], err);
            io_free_string(&string);
            return err;
        }
        err = parser_parse(&parser, &unit.program);
        if (err != 0)
        {
            fprintf(stderr, "Error parsing file %s: %d\n", filenames[i], err);
            parser_free_program(&unit.program);
            io_free_string(&string);
            return err;
        }

        DYNARRAY_PUSH(*mod, unit, m_unit_t);
    }

    return 0;
}

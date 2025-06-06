#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "parser.h"
#include "io.h"
#include "benchmark.h"

#define MAX_THREADS 10

typedef struct
{
    char *filename;
    program_t program;
} m_unit_t;

typedef struct
{
    char **filenames;
    m_unit_t *output;
    size_t from;
    size_t to;
} thread_data_t;

pthread_t threads[MAX_THREADS] = {0};
thread_data_t thread_data[MAX_THREADS] = {0};

void *module_parse_files(void *arg);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file1> <file2> ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    argv++;
    argc--;

    double start_time = benchmark_get_time();

    size_t modules_per_thread = argc / MAX_THREADS;
    if (modules_per_thread == 0)
    {
        modules_per_thread = 1;
    }

    printf("[INFO]: Running Multi-threaded parser\n");
    printf("[INFO]: Number of files: %d\n", argc);
    printf("[INFO]: Number of threads: %d\n", MAX_THREADS);
    printf("[INFO]: Number of modules per thread: %zu\n", modules_per_thread);

    m_unit_t *output = malloc(argc * sizeof(m_unit_t));
    if (output == NULL)
    {
        fprintf(stderr, "Error allocating memory for output\n");
        return EXIT_FAILURE;
    }

    // Initialize all programs to zero
    for (int i = 0; i < argc; i++)
    {
        output[i].program = (program_t){0};
    }

    size_t thread_count = 0;
    for (int i = 0; i < argc; i += modules_per_thread)
    {
        size_t from = i;
        size_t to = i + modules_per_thread;
        if (to > (size_t)argc)
        {
            to = argc;
        }

        thread_data[thread_count] = (thread_data_t){
            .filenames = argv,
            .from = from,
            .to = to,
            .output = output};

        int err = pthread_create(&threads[thread_count], NULL, module_parse_files, &thread_data[thread_count]);
        if (err != 0)
        {
            fprintf(stderr, "Error creating thread: %d\n", err);
            // Clean up already created threads
            for (size_t j = 0; j < thread_count; j++)
            {
                pthread_join(threads[j], NULL);
            }
            free(output);
            return EXIT_FAILURE;
        }
        thread_count++;
    }

    // Wait for all threads to complete
    for (size_t i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i], NULL);
    }

    double end_time = benchmark_get_time();
    double duration = end_time - start_time;

    fprintf(stderr, "[INFO]: Parsed %d files successfully.\n", argc);
    printf("[INFO]: Parsing time: %f seconds\n", duration);

    // Clean up all programs
    for (int i = 0; i < argc; i++)
    {
        parser_free_program(&output[i].program);
    }

    free(output);
    return EXIT_SUCCESS;
}

void *module_parse_files(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;

    for (size_t i = data->from; i < data->to; i++)
    {
        char *filename = data->filenames[i];
        program_t *program = &data->output[i].program;
        io_str_t str = {0};

        int err = io_load_file_into_memory(filename, &str);
        if (err != 0)
        {
            fprintf(stderr, "[ERROR]: Failed to load file %s\n", filename);
            continue;
        }

        parser_t parser = {0};
        err = parser_init(&parser, str.data, str.size);
        if (err != 0)
        {
            fprintf(stderr, "[ERROR]: Failed to initialize parser\n");
            free(str.data);
            continue;
        }

        err = parser_parse(&parser, program);
        if (err != 0)
        {
            fprintf(stderr, "[ERROR]: Failed to parse file %s\n", filename);
            free(str.data);
            continue;
        }

        free(str.data);
    }

    return NULL;
}

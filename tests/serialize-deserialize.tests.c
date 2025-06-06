#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "serialize.h"
#include "parser.h"

int main(void)
{
    fprintf(stdout, "[TEST] serialize-deserialize\n");

    int fd = open("test.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        fprintf(stderr, "[FAIL] Failed to open file: %s\n", strerror(errno));
        return 1;
    }

    serializer_t serializer;

    int err = serializer_init(&serializer, fd);
    if (err)
    {
        fprintf(stderr, "[FAIL] Failed to initialize serializer: %d\n", err);
        return 1;
    }

    char *program_str = "(+ 1 2 3)";
    size_t program_len = strlen(program_str);

    parser_t parser = {0};
    program_t program = {0};
    err = parser_init(&parser, program_str, program_len);
    if (err)
    {
        fprintf(stderr, "[FAIL] Failed to initialize parser: %d\n", err);
        return 1;
    }

    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] Failed to parse program: %d\n", err);
        return 1;
    }

    err = serializer_serialize(&serializer, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] Failed to serialize program: %d\n", err);
        return 1;
    }

    close(fd);

    fd = open("test.bin", O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "[FAIL] Failed to open file: %s\n", strerror(errno));
        return 1;
    }

    program_t program2 = {0};
    deserializer_t deserializer = {0};
    err = deserializer_init(&deserializer, fd);
    if (err)
    {
        fprintf(stderr, "[FAIL] Failed to initialize deserializer: %d\n", err);
        return 1;
    }

    err = deserializer_deserialize(&deserializer, &program2);
    if (err)
    {
        fprintf(stderr, "[FAIL] Failed to deserialize program: %d\n", err);
        return 1;
    }

    int equals = __program_equals(&program, &program2);

    parser_free_program(&program);
    parser_free_program(&program2);

    err = close(fd);
    if (err)
    {
        fprintf(stderr, "[FAIL] Failed to close file: %d\n", err);
        return 1;
    }

    // delete file
    err = unlink("test.bin");
    if (err)
    {
        fprintf(stderr, "[FAIL] Failed to delete file: %d\n", err);
        return 1;
    }

    if (!equals)
    {
        fprintf(stdout, "[FAIL] Programs are not equal\n");
        return 1;
    }

    fprintf(stdout, "[OK] Programs are equal\n");

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "serialize.h"
#include "parser.h"

int main(void)
{
    fprintf(stdout, "[TEST] server:serial-over-the-wire\n");
    // create the server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        fprintf(stderr, "[FAIL] server:Failed to create server socket: %s\n", strerror(errno));
        return 1;
    }

    // bind the server socket to the port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind the server socket to the port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        fprintf(stderr, "[FAIL] server:Failed to bind server socket: %s\n", strerror(errno));
        return 1;
    }

    // listen for connections
    if (listen(server_fd, 10) == -1)
    {
        fprintf(stderr, "[FAIL] server:Failed to listen for connections: %s\n", strerror(errno));
        return 1;
    }

    char *input = "(- 32 3.14 (- 9 2))";
    size_t input_len = strlen(input);

    parser_t parser = {0};
    int err = parser_init(&parser, input, input_len);
    if (err)
    {
        fprintf(stderr, "[FAIL] server:Failed to initialize parser: %d\n", err);
        return 1;
    }

    program_t program = {0};
    err = parser_parse(&parser, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] server:Failed to parse program: %d\n", err);
        return 1;
    }

    // accept the client connection
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1)
    {
        fprintf(stderr, "[FAIL] server:Failed to accept client connection: %s\n", strerror(errno));
        return 1;
    }

    serializer_t serializer = {0};
    err = serializer_init(&serializer, client_fd);
    if (err)
    {
        fprintf(stderr, "[FAIL] server:Failed to initialize serializer: %d\n", err);
        return 1;
    }

    // send the program to the client
    err = serializer_serialize(&serializer, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] server:Failed to serialize program: %d\n", err);
        return 1;
    }

    sleep(1);

    deserializer_t deserializer = {0};
    err = deserializer_init(&deserializer, client_fd);
    if (err)
    {
        fprintf(stderr, "[FAIL] server:Failed to initialize deserializer: %d\n", err);
        return 1;
    }

    program_t program2 = {0};
    err = deserializer_deserialize(&deserializer, &program2);
    if (err)
    {
        fprintf(stderr, "[FAIL] server:Failed to deserialize program: %d\n", err);
        return 1;
    }

    // close the client connection
    close(client_fd);

    // close the server socket
    close(server_fd);

    int equals = __program_equals(&program, &program2);

    if (!equals)
    {
        fprintf(stdout, "[FAIL] server:Programs are not equal\n");
        parser_free_program(&program);
        parser_free_program(&program2);

        return 1;
    }

    // delete the program
    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] server:Failed to free program: %d\n", err);
        return 1;
    }

    err = parser_free_program(&program2);
    if (err)
    {
        fprintf(stderr, "[FAIL] server:Failed to free program: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[OK] server:Serial-over-the-wire test passed\n");

    return 0;
}

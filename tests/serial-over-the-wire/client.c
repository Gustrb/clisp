#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
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
    fprintf(stdout, "[TEST] client:serial-over-the-wire\n");

    // create the client socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
    {
        fprintf(stderr, "[FAIL] client:Failed to create client socket: %s\n", strerror(errno));
        return 1;
    }

    // connect to the server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        fprintf(stderr, "[FAIL] client:Failed to connect to server: %s\n", strerror(errno));
        return 1;
    }

    program_t program = {0};
    deserializer_t deserializer = {0};

    int err = deserializer_init(&deserializer, client_fd);
    if (err)
    {
        fprintf(stderr, "[FAIL] client:Failed to initialize deserializer: %d\n", err);
        return 1;
    }

    err = deserializer_deserialize(&deserializer, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] client:Failed to deserialize program: %d\n", err);
        return 1;
    }

    serializer_t serializer = {0};
    err = serializer_init(&serializer, client_fd);
    if (err)
    {
        fprintf(stderr, "[FAIL] client:Failed to initialize serializer: %d\n", err);
        return 1;
    }

    err = serializer_serialize(&serializer, &program);
    if (err)
    {
        fprintf(stderr, "[FAIL] client:Failed to serialize program: %d\n", err);
        return 1;
    }

    err = parser_free_program(&program);
    if (err)
    {
        fprintf(stderr, "[FAIL] client:Failed to free program: %d\n", err);
        return 1;
    }

    fprintf(stdout, "[OK] client:Serial-over-the-wire test passed\n");

    return 0;
}

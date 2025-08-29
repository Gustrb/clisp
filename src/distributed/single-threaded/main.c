#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#include "parser.h"
#include "dynarray.h"
#include "io.h"
#include "benchmark.h"
#include "serialize.h"
#include "raft-c/raft.h"

// Configuration
#define DEFAULT_PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096

typedef struct {
    int node_id;
    char *address;
    int port;
    int socket_fd;
    struct sockaddr_in addr;
} node_info_t;

typedef struct {
    raft_server_t *raft;
    int node_id;
    char *address;
    int port;
    int socket_fd;
    int client_socket;
    node_info_t *nodes;
    size_t num_nodes;
    program_t *parsed_programs;
    size_t num_programs;
    pthread_mutex_t mutex;
} distributed_parser_t;

// Forward declarations
int setup_network(distributed_parser_t *parser);
int setup_raft(distributed_parser_t *parser);
void cleanup_parser(distributed_parser_t *parser);
int parse_files_distributed(distributed_parser_t *parser, char **filenames, size_t count);

// Raft callbacks
int send_requestvote(raft_server_t *raft, void *user_data, raft_node_t *node, msg_requestvote_t *msg);
int send_appendentries(raft_server_t *raft, void *user_data, raft_node_t *node, msg_appendentries_t *msg);
int applylog(raft_server_t *raft, void *user_data, raft_entry_t *entry, raft_index_t entry_idx);
int persist_vote(raft_server_t *raft, void *user_data, raft_node_id_t vote);
int persist_term(raft_server_t *raft, void *user_data, raft_term_t term, raft_node_id_t vote);
int log_offer(raft_server_t *raft, void *user_data, raft_entry_t *entry, raft_index_t entry_idx);
int log_poll_callback(raft_server_t *raft, void *user_data, raft_entry_t *entry, raft_index_t entry_idx);
int log_pop_callback(raft_server_t *raft, void *user_data, raft_entry_t *entry, raft_index_t entry_idx);
int log_clear_callback(raft_server_t *raft, void *user_data, raft_entry_t *entry, raft_index_t entry_idx);

int setup_network(distributed_parser_t *parser)
{
    // Create socket
    parser->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (parser->socket_fd == -1) {
        perror("Error creating socket");
        return -1;
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(parser->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting socket options");
        close(parser->socket_fd);
        return -1;
    }

    // Setup address structure
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(parser->address);
    addr.sin_port = htons(parser->port);

    // Bind socket
    if (bind(parser->socket_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Error binding socket");
        close(parser->socket_fd);
        return -1;
    }

    // Listen for connections
    if (listen(parser->socket_fd, MAX_CLIENTS) < 0) {
        perror("Error listening on socket");
        close(parser->socket_fd);
        return -1;
    }

    printf("[INFO]: Network setup complete - listening on port %d\n", parser->port);
    return 0;
}

int setup_raft(distributed_parser_t *parser)
{
    // Create new Raft server
    parser->raft = raft_new();
    if (!parser->raft) {
        fprintf(stderr, "Error: Failed to create Raft server\n");
        return -1;
    }

    // Setup Raft callbacks
    raft_cbs_t callbacks = {0};
    callbacks.send_requestvote = send_requestvote;
    callbacks.send_appendentries = send_appendentries;
    callbacks.applylog = applylog;
    callbacks.persist_vote = persist_vote;
    callbacks.persist_term = persist_term;
    callbacks.log_offer = log_offer;
    callbacks.log_poll = log_poll_callback;
    callbacks.log_pop = log_pop_callback;
    callbacks.log_clear = log_clear_callback;

    // Set callbacks
    raft_set_callbacks(parser->raft, &callbacks, parser);

    // Add this node to the cluster
    raft_node_t *node = raft_add_node(parser->raft, parser, parser->node_id, 1);
    if (!node) {
        fprintf(stderr, "Error: Failed to add node to Raft cluster\n");
        raft_free(parser->raft);
        return -1;
    }

    // Set timeouts
    raft_set_election_timeout(parser->raft, 1000); // 1 second
    raft_set_request_timeout(parser->raft, 200);   // 200ms

    printf("[INFO]: Raft setup complete - node %d added to cluster\n", parser->node_id);
    return 0;
}

void cleanup_parser(distributed_parser_t *parser)
{
    if (parser->raft) {
        raft_free(parser->raft);
        parser->raft = NULL;
    }

    if (parser->socket_fd >= 0) {
        close(parser->socket_fd);
        parser->socket_fd = -1;
    }

    if (parser->client_socket >= 0) {
        close(parser->client_socket);
        parser->client_socket = -1;
    }

    // Free parsed programs
    if (parser->parsed_programs) {
        for (size_t i = 0; i < parser->num_programs; ++i) {
            parser_free_program(&parser->parsed_programs[i]);
        }
        free(parser->parsed_programs);
        parser->parsed_programs = NULL;
    }

    // Free nodes array
    if (parser->nodes) {
        free(parser->nodes);
        parser->nodes = NULL;
    }

    // Destroy mutex
    pthread_mutex_destroy(&parser->mutex);

    printf("[INFO]: Parser cleanup complete\n");
}

int parse_files_distributed(distributed_parser_t *parser, char **filenames, size_t count)
{
    // For now, just parse files locally like the single-threaded version
    // In a full implementation, this would distribute the work across nodes
    
    parser_t parser_instance = {0};
    int err = 0;
    
    // Allocate space for parsed programs
    parser->parsed_programs = malloc(count * sizeof(program_t));
    if (!parser->parsed_programs) {
        fprintf(stderr, "Error: Failed to allocate memory for parsed programs\n");
        return -1;
    }
    parser->num_programs = count;

    for (size_t i = 0; i < count; ++i) {
        program_t *program = &parser->parsed_programs[i];
        
        // Load file into memory
        io_str_t string = {0};
        err = io_load_file_into_memory(filenames[i], &string);
        if (err != 0) {
            fprintf(stderr, "Error loading file %s: %d\n", filenames[i], err);
            return err;
        }

        // Initialize parser
        err = parser_init(&parser_instance, string.data, string.size);
        if (err != 0) {
            fprintf(stderr, "Error initializing parser for file %s: %d\n", filenames[i], err);
            io_free_string(&string);
            return err;
        }

        // Parse the file
        err = parser_parse(&parser_instance, program);
        if (err != 0) {
            fprintf(stderr, "Error parsing file %s: %d\n", filenames[i], err);
            parser_free_program(program);
            io_free_string(&string);
            return err;
        }

        printf("[INFO]: Successfully parsed file %s (%zu forms)\n", filenames[i], program->size);
        io_free_string(&string);
    }

    return 0;
}

// Raft callback implementations (stubs for now)
int send_requestvote(raft_server_t *raft, void *user_data, raft_node_t *node, msg_requestvote_t *msg)
{
    // TODO: Implement network sending of request vote
    printf("[RAFT]: Sending request vote to node\n");
    return 0;
}

int send_appendentries(raft_server_t *raft, void *user_data, raft_node_t *node, msg_appendentries_t *msg)
{
    // TODO: Implement network sending of append entries
    printf("[RAFT]: Sending append entries to node\n");
    return 0;
}

int applylog(raft_server_t *raft, void *user_data, raft_entry_t *entry, raft_index_t entry_idx)
{
    // TODO: Apply log entry to state machine
    printf("[RAFT]: Applying log entry %ld\n", entry_idx);
    return 0;
}

int persist_vote(raft_server_t *raft, void *user_data, raft_node_id_t vote)
{
    // TODO: Persist vote to disk
    printf("[RAFT]: Persisting vote for node %d\n", vote);
    return 0;
}

int persist_term(raft_server_t *raft, void *user_data, raft_term_t term, raft_node_id_t vote)
{
    // TODO: Persist term to disk
    printf("[RAFT]: Persisting term %ld, vote %d\n", term, vote);
    return 0;
}

int log_offer(raft_server_t *raft, void *user_data, raft_entry_t *entry, raft_index_t entry_idx)
{
    // TODO: Add entry to log
    printf("[RAFT]: Offering log entry %ld\n", entry_idx);
    return 0;
}

int log_poll_callback(raft_server_t *raft, void *user_data, raft_entry_t *entry, raft_index_t entry_idx)
{
    // TODO: Remove oldest entry from log
    printf("[RAFT]: Polling log entry %ld\n", entry_idx);
    return 0;
}

int log_pop_callback(raft_server_t *raft, void *user_data, raft_entry_t *entry, raft_index_t entry_idx)
{
    // TODO: Remove youngest entry from log
    printf("[RAFT]: Popping log entry %ld\n", entry_idx);
    return 0;
}

int log_clear_callback(raft_server_t *raft, void *user_data, raft_entry_t *entry, raft_index_t entry_idx)
{
    // TODO: Clear log entry
    printf("[RAFT]: Clearing log entry %ld\n", entry_idx);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <node_id> <port> [file1] [file2] ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[INFO]: Starting Distributed Single-threaded Parser\n");
    
    int node_id = atoi(argv[1]);
    int port = atoi(argv[2]);
    
    printf("[INFO]: Node ID: %d, Port: %d\n", node_id, port);

    distributed_parser_t parser = {0};
    parser.node_id = node_id;
    parser.port = port;
    parser.address = "127.0.0.1"; // Default to localhost for now
    parser.socket_fd = -1;
    parser.client_socket = -1;
    
    // Initialize mutex
    if (pthread_mutex_init(&parser.mutex, NULL) != 0) {
        fprintf(stderr, "Error: Failed to initialize mutex\n");
        return EXIT_FAILURE;
    }

    // Setup network
    if (setup_network(&parser) != 0) {
        fprintf(stderr, "Error: Failed to setup network\n");
        cleanup_parser(&parser);
        return EXIT_FAILURE;
    }

    // Setup Raft
    if (setup_raft(&parser) != 0) {
        fprintf(stderr, "Error: Failed to setup Raft\n");
        cleanup_parser(&parser);
        return EXIT_FAILURE;
    }

    printf("[INFO]: Distributed parser initialized successfully\n");

    // If files are provided, parse them
    if (argc > 3) {
        char **filenames = &argv[3];
        size_t count = argc - 3;
        
        printf("[INFO]: Parsing %zu files\n", count);
        
        double start_time = benchmark_get_time();
        
        if (parse_files_distributed(&parser, filenames, count) != 0) {
            fprintf(stderr, "Error: Failed to parse files\n");
            cleanup_parser(&parser);
            return EXIT_FAILURE;
        }
        
        double end_time = benchmark_get_time();
        double duration = end_time - start_time;
        
        printf("[INFO]: Parsing completed in %f seconds\n", duration);
    }

    // Main event loop would go here
    printf("[INFO]: Entering main event loop...\n");
    
    // For now, just sleep a bit to simulate work
    sleep(5);

    cleanup_parser(&parser);
    return EXIT_SUCCESS;
}

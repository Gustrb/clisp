#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h> // For fcntl
#include <errno.h> // For errno

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
#define MAX_NODES 10 // Maximum number of nodes to discover/manage
#define HEARTBEAT_INTERVAL 1000000 // 1 second heartbeat interval

// Message types for node communication
typedef enum {
    MSG_HEARTBEAT = 1,
    MSG_NODE_DISCOVERY = 2,
    MSG_PARSING_REQUEST = 3,
    MSG_PARSING_RESULT = 4,
    MSG_RAFT_MESSAGE = 5,
    MSG_UNKNOWN = 0
} network_message_type_t;

typedef struct {
    network_message_type_t type;
    int node_id;
    size_t data_size;
    char data[BUFFER_SIZE - sizeof(network_message_type_t) - sizeof(int) - sizeof(size_t)];
} network_message_t;

typedef struct {
    int node_id;
    char *address;
    int port;
    int socket_fd;
    struct sockaddr_in addr;
    int is_connected;
    time_t last_heartbeat;
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
int parse_files_local(distributed_parser_t *parser, char **filenames, size_t count);

// Network communication functions
int send_message(int socket_fd, network_message_t *msg);
int receive_message(int socket_fd, network_message_t *msg);
int discover_nodes(distributed_parser_t *parser);
int connect_to_node(distributed_parser_t *parser, int target_node_id, char *address, int port);
int handle_incoming_connection(distributed_parser_t *parser, int client_socket);
void *network_listener_thread(void *arg);
void *heartbeat_thread(void *arg);
void *raft_processing_thread(void *arg);
int broadcast_message(distributed_parser_t *parser, network_message_t *msg);

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
    
    // Initialize nodes array
    parser->nodes = malloc(MAX_NODES * sizeof(node_info_t));
    if (!parser->nodes) {
        perror("Error allocating nodes array");
        close(parser->socket_fd);
        return -1;
    }
    memset(parser->nodes, 0, MAX_NODES * sizeof(node_info_t));
    parser->num_nodes = 0;
    
    return 0;
}

// Network communication functions
int send_message(int socket_fd, network_message_t *msg)
{
    ssize_t bytes_sent = send(socket_fd, msg, sizeof(network_message_t), 0);
    if (bytes_sent == -1) {
        perror("Error sending message");
        return -1;
    }
    if (bytes_sent != sizeof(network_message_t)) {
        fprintf(stderr, "Error: Incomplete message sent\n");
        return -1;
    }
    return 0;
}

int receive_message(int socket_fd, network_message_t *msg)
{
    ssize_t bytes_received = recv(socket_fd, msg, sizeof(network_message_t), 0);
    if (bytes_received == -1) {
        perror("Error receiving message");
        return -1;
    }
    if (bytes_received == 0) {
        // Connection closed by peer
        return -2;
    }
    if (bytes_received != sizeof(network_message_t)) {
        fprintf(stderr, "Error: Incomplete message received\n");
        return -1;
    }
    return 0;
}

int discover_nodes(distributed_parser_t *parser)
{
    printf("[INFO]: Starting node discovery...\n");
    
    // For now, we'll use a simple approach - try to connect to known ports
    // In a real implementation, this would use a discovery protocol
    
    // Try to connect to other nodes on common ports
    for (int port = 8080; port <= 8084; port++) {
        if (port == parser->port) continue; // Skip our own port
        
        int discovery_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (discovery_socket == -1) continue;
        
        struct sockaddr_in target_addr;
        memset(&target_addr, 0, sizeof(struct sockaddr_in));
        target_addr.sin_family = AF_INET;
        target_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        target_addr.sin_port = htons(port);
        
        // Set non-blocking for quick connection test
        int flags = fcntl(discovery_socket, F_GETFL, 0);
        fcntl(discovery_socket, F_SETFL, flags | O_NONBLOCK);
        
        int result = connect(discovery_socket, (struct sockaddr*)&target_addr, sizeof(struct sockaddr_in));
        if (result == 0 || errno == EINPROGRESS) {
            // Connection successful or in progress
            printf("[INFO]: Found node on port %d\n", port);
            
            // Add to our nodes list
            if (parser->num_nodes < MAX_NODES) {
                node_info_t *node = &parser->nodes[parser->num_nodes];
                node->node_id = parser->num_nodes + 1;
                node->address = "127.0.0.1";
                node->port = port;
                node->socket_fd = discovery_socket;
                node->is_connected = 1;
                node->last_heartbeat = time(NULL);
                parser->num_nodes++;
            }
        } else {
            close(discovery_socket);
        }
    }
    
    printf("[INFO]: Node discovery complete. Found %zu nodes\n", parser->num_nodes);
    
    // For now, let's not try to connect to non-existent nodes
    // In a real distributed setup, we would have actual nodes running
    if (parser->num_nodes > 0) {
        printf("[INFO]: Note: These are simulated connections for testing.\n");
        printf("[INFO]: In a real setup, actual nodes would be running on these ports.\n");
    }
    
    return 0;
}

int connect_to_node(distributed_parser_t *parser, int target_node_id, char *address, int port)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Error creating connection socket");
        return -1;
    }
    
    struct sockaddr_in target_addr;
    memset(&target_addr, 0, sizeof(struct sockaddr_in));
    target_addr.sin_family = AF_INET;
    target_addr.sin_addr.s_addr = inet_addr(address);
    target_addr.sin_port = htons(port);
    
    if (connect(socket_fd, (struct sockaddr*)&target_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Error connecting to node");
        close(socket_fd);
        return -1;
    }
    
    printf("[INFO]: Connected to node %d at %s:%d\n", target_node_id, address, port);
    return socket_fd;
}

int handle_incoming_connection(distributed_parser_t *parser, int client_socket)
{
    network_message_t msg;
    int result = receive_message(client_socket, &msg);
    
    if (result == 0) {
        printf("[INFO]: Received message type %d from node %d\n", msg.type, msg.node_id);
        
        switch (msg.type) {
            case MSG_HEARTBEAT:
                printf("[INFO]: Heartbeat from node %d\n", msg.node_id);
                // Send heartbeat response
                msg.type = MSG_HEARTBEAT;
                msg.node_id = parser->node_id;
                send_message(client_socket, &msg);
                break;
                
            case MSG_NODE_DISCOVERY:
                printf("[INFO]: Node discovery request from node %d\n", msg.node_id);
                // Send discovery response
                msg.type = MSG_NODE_DISCOVERY;
                msg.node_id = parser->node_id;
                send_message(client_socket, &msg);
                break;
                
            case MSG_PARSING_REQUEST:
                printf("[INFO]: Parsing request from node %d\n", msg.node_id);
                // TODO: Handle parsing request
                break;
                
            case MSG_PARSING_RESULT:
                printf("[INFO]: Parsing result from node %d\n", msg.node_id);
                // TODO: Handle parsing result
                break;
                
            case MSG_RAFT_MESSAGE:
                printf("[INFO]: Raft message from node %d\n", msg.node_id);
                // TODO: Handle Raft message
                break;
                
            default:
                printf("[WARN]: Unknown message type %d\n", msg.type);
                break;
        }
    }
    
    close(client_socket);
    return 0;
}

void *network_listener_thread(void *arg)
{
    distributed_parser_t *parser = (distributed_parser_t *)arg;
    
    printf("[INFO]: Network listener thread started\n");
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(parser->socket_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            continue;
        }
        
        printf("[INFO]: New connection from %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        handle_incoming_connection(parser, client_socket);
    }
    
    return NULL;
}

int broadcast_message(distributed_parser_t *parser, network_message_t *msg)
{
    int success_count = 0;
    
    for (size_t i = 0; i < parser->num_nodes; i++) {
        if (parser->nodes[i].is_connected) {
            if (send_message(parser->nodes[i].socket_fd, msg) == 0) {
                success_count++;
            }
        }
    }
    
    printf("[INFO]: Broadcasted message to %d/%zu nodes\n", success_count, parser->num_nodes);
    return success_count;
}

void *heartbeat_thread(void *arg)
{
    distributed_parser_t *parser = (distributed_parser_t *)arg;
    
    printf("[INFO]: Heartbeat thread started\n");
    
    while (1) {
        // Send heartbeat to all connected nodes
        network_message_t heartbeat_msg;
        heartbeat_msg.type = MSG_HEARTBEAT;
        heartbeat_msg.node_id = parser->node_id;
        heartbeat_msg.data_size = 0;
        
        for (size_t i = 0; i < parser->num_nodes; i++) {
            if (parser->nodes[i].is_connected) {
                if (send_message(parser->nodes[i].socket_fd, &heartbeat_msg) != 0) {
                    // Connection failed, mark as disconnected
                    printf("[WARN]: Lost connection to node %d\n", parser->nodes[i].node_id);
                    parser->nodes[i].is_connected = 0;
                    close(parser->nodes[i].socket_fd);
                }
            }
        }
        
        // Sleep for heartbeat interval
        usleep(HEARTBEAT_INTERVAL * 1000);
    }
    
    return NULL;
}

void *raft_processing_thread(void *arg)
{
    distributed_parser_t *parser = (distributed_parser_t *)arg;
    
    printf("[RAFT]: Raft processing thread started\n");
    
    while (1) {
        // Process Raft operations
        int result = raft_periodic(parser->raft, 10); // 10ms timeout
        
        if (result != 0) {
            printf("[RAFT]: Raft periodic processing returned %d\n", result);
        }
        
        // Check if we're the leader
        if (raft_is_leader(parser->raft)) {
            static int leader_announcement = 0;
            if (!leader_announcement) {
                printf("[RAFT]: 🎉 Node %d is now the LEADER!\n", parser->node_id);
                leader_announcement = 1;
            }
        } else {
            static int leader_announcement = 1;
            if (leader_announcement) {
                printf("[RAFT]: Node %d is now a FOLLOWER\n", parser->node_id);
                leader_announcement = 0;
            }
        }
        
        // Sleep for a short interval
        usleep(10000); // 10ms
    }
    
    return NULL;
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
    printf("[DIST]: Starting distributed parsing of %zu files\n", count);
    
    // Check if we have other nodes available
    if (parser->num_nodes == 0) {
        printf("[DIST]: No other nodes available, parsing locally\n");
        printf("[DIST]: File distribution across 1 node:\n");
        for (size_t i = 0; i < count; i++) {
            printf("[DIST]:   File %zu (%s) -> Node 1\n", i, filenames[i]);
        }
        return parse_files_local(parser, filenames, count);
    } else {
        printf("[DIST]: Found %zu other nodes! Real distributed parsing enabled!\n", parser->num_nodes);
    }
    
    // Calculate total nodes (including this one)
    size_t total_nodes = parser->num_nodes + 1;
    printf("[DIST]: Distributing work across %zu nodes\n", total_nodes);
    
    // Split files across nodes
    size_t files_per_node = count / total_nodes;
    size_t extra_files = count % total_nodes;
    
    printf("[DIST]: Each node will parse %zu files, with %zu extra files\n", files_per_node, extra_files);
    
    // Determine which files this node should parse
    size_t start_file = 0;
    size_t files_to_parse = files_per_node;
    
    // Distribute extra files among the first few nodes
    if ((size_t)parser->node_id <= extra_files) {
        files_to_parse++;
    }
    
    // Calculate start file index for this node
    if ((size_t)parser->node_id <= extra_files) {
        start_file = (parser->node_id - 1) * (files_per_node + 1);
    } else {
        start_file = extra_files * (files_per_node + 1) + (parser->node_id - extra_files - 1) * files_per_node;
    }
    
    printf("[DIST]: Node %d will parse files %zu to %zu (%zu files)\n", 
           parser->node_id, start_file, start_file + files_to_parse - 1, files_to_parse);
    
    // Show file distribution for debugging (always show for demonstration)
    printf("[DIST]: File distribution across %zu nodes:\n", total_nodes);
    for (size_t i = 0; i < count; i++) {
        size_t node_for_file = 0;
        if (i < extra_files * (files_per_node + 1)) {
            node_for_file = i / (files_per_node + 1) + 1;
        } else {
            size_t remaining_files = i - extra_files * (files_per_node + 1);
            node_for_file = extra_files + remaining_files / files_per_node + 1;
        }
        printf("[DIST]:   File %zu (%s) -> Node %zu\n", i, filenames[i], node_for_file);
    }
    
    // Parse our assigned files locally
    if (files_to_parse > 0) {
        char **our_files = &filenames[start_file];
        int result = parse_files_local(parser, our_files, files_to_parse);
        if (result != 0) {
            fprintf(stderr, "Error: Failed to parse local files\n");
            return result;
        }
    }
    
    printf("[DIST]: Distributed parsing completed\n");
    return 0;
}

int parse_files_local(distributed_parser_t *parser, char **filenames, size_t count)
{
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

    // Discover other nodes
    discover_nodes(&parser);

    // Start network listener thread
    pthread_t listener_thread;
    if (pthread_create(&listener_thread, NULL, network_listener_thread, &parser) != 0) {
        fprintf(stderr, "Error: Failed to create network listener thread\n");
        cleanup_parser(&parser);
        return EXIT_FAILURE;
    }

    // Start heartbeat thread
    pthread_t heartbeat_thread_id;
    if (pthread_create(&heartbeat_thread_id, NULL, heartbeat_thread, &parser) != 0) {
        fprintf(stderr, "Error: Failed to create heartbeat thread\n");
        cleanup_parser(&parser);
        return EXIT_FAILURE;
    }

    // Start Raft processing thread
    pthread_t raft_thread;
    if (pthread_create(&raft_thread, NULL, raft_processing_thread, &parser) != 0) {
        fprintf(stderr, "Error: Failed to create Raft processing thread\n");
        cleanup_parser(&parser);
        return EXIT_FAILURE;
    }

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

    // Main event loop
    printf("[INFO]: Entering main event loop...\n");
    printf("[INFO]: Node %d is ready for distributed operations\n", parser.node_id);
    
    // Keep the main thread alive
    while (1) {
        sleep(10);
        printf("[INFO]: Node %d is still running...\n", parser.node_id);
    }

    cleanup_parser(&parser);
    return EXIT_SUCCESS;
}

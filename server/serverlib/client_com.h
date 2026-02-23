
#pragma once
// concerning syncing of clients to the game state

static const int MAX_CLIENTS = 8;

typedef struct client_t client_t;

void client_com_init(void);

/**
 * @brief Processes queued messages from clients
 */
void server_process_queue(void);

/**
 * @brief Set the server connect clients count;
 * should really only be used in tests
 * 
 * @param count The number of connected clients to set
 */
void server_set_connected_clients_count(int count);

/**
 * @brief Get the number of connected clients
 * @return The number of connected clients
 */
unsigned int server_get_connected_clients_count(void);

/**
 * @brief Get a client by index
 * @param index The index of the client to get
 * @return The client at the given index, or NULL if out of bounds
 */
client_t* server_get_client_by_index(unsigned int index);

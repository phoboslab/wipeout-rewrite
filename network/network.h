
#pragma once

#include "network_types.h"

#if defined(WIN32)
#include <WinSock2.h>
#include <win_defines.h>
#else
#include <sys/socket.h>
#define INVALID_SOCKET -1
#endif

static int WIPEOUT_PORT = 8000;
static int WIPEOUT_CLIENT_PORT = 8001;
static int CLIENT_SOCKET_TIMEOUT = 3000; // 3 seconds

typedef struct {
    const char* command;
    struct sockaddr_storage dest_addr;
} msg_queue_item_t;

#if defined(WIN32)
/**
 * @brief Initialize the Winsock library
 * 
 * @return true if initialization was successful
 */
bool system_init_winsock(void);

/**
 * @brief Cleanup the Winsock library
 */
void system_cleanup_winsock(void);
#endif

/**
 * @brief Check if the current socket is valid and bound
 * 
 * @return true if the socket is valid and bound
 */
bool network_has_bound_ip_socket(void);

/**
 * @brief Set a bound ip socket; really
 * should only be used for testing
 * 
 * @param sockfd the socket fd
 */
void network_set_bound_ip_socket(int sockfd);

/**
 * @brief Get the currently bound ip socket;
 * callers should check if the socket is valid first
 * 
 * @return the socket fd
 */
int network_get_bound_ip_socket(void);

/**
 * @brief Get a client socket for sending and receiving packets
 * 
 * Returns -1 (INVALID_SOCKET) if the socket could not be created
 * 
 * @return int the socket fd
 */
int network_get_client_socket(void);

/**
 * @brief Close a socket. The socket fd will be set to INVALID_SOCKET
 * on close.
 * 
 * @param sockfd a pointer to the socket fd
 */
void network_close_socket(int* sockfd);

/**
 * @brief Bind a socket to a specific IP address and port to
 * listen for incoming packets.
 * 
 * @param sockfd the socket file descriptor
 * @param ip_addr the IP address to bind to
 * @param port the port to bind to
 */
bool network_bind_socket(int sockfd, char *ip_addr, char* port);

bool network_get_packet(void);

/**
 * @brief Send a packet of data to a specific destination
 * 
 * @param sockfd the socket file descriptor
 * @param length the length of the data to send
 * @param data the data to send
 * @param dest_net the destination netadr_t
 */
void network_send_packet(int sockfd, int length, const void* data, netadr_t dest_net);

/**
 * @brief Send a command to the server or to a client
 */
void network_send_command(const char* command, netadr_t dest);

/**
 * @brief clears the current message queue
 */
void network_clear_msg_queue(void);

/**
 * @brief gets the size of the message queue
 */
int network_get_msg_queue_size(void);

/**
 * @brief pop out the first message queue item
 */
void network_popleft_msg_queue(void);

/**
 * @brief get the first message queue item
 * 
 * @param item pointer to the item
 * 
 * @return true if there is an item, false otherwise
 */
bool network_get_msg_queue_item(msg_queue_item_t *item);

/**
 * @brief sleep for a duration, or until we have socket activity
 * 
 * @param msec duration in milliseconds
 * @return 0 if network is not available, or 1 if so
 */
int network_sleep(int msec);

/**
 * @brief get the local subnet for this network participant
 * 
 * @param[in,out] subnet the subnet address
 * @param len the length of the subnet address
 */
void network_get_my_ip(char* subnet, size_t len);

broadcast_list_t network_get_broadcast_addresses(void);

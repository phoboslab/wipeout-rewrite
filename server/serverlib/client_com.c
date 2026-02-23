
#include "client_com.h"

#include <network_types.h>
#include <addr_conversions.h>

#include <msg.h>
#include <network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#if defined(WIN32)
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include <ServerInfo.pb-c.h>

typedef enum { DISCONNECTED, CONNECTED } connect_state_t;

struct client_t {
    const char *name;
    struct sockaddr_in addr; // client address
};

unsigned int current_client_count = 0; // number of connected clients
static client_t* clients = NULL; // array of clients


void client_com_init(void) {
    clients = malloc(sizeof(client_t) * MAX_CLIENTS);
    if (!clients) {
        fprintf(stderr, "Failed to allocate memory for clients\n");
        exit(EXIT_FAILURE);
    }
    current_client_count = 0;
}


static void server_connect_client(struct sockaddr_in net_addr) {

    if(!clients) {
        fprintf(stderr, "Client array not initialized\n");

        const char* response = "connect_failed";
        network_send_packet(network_get_bound_ip_socket(), strlen(response), response, net_addr);
        return;
    }

    if (current_client_count >= MAX_CLIENTS) {
        fprintf(stderr, "Cannot connect client: max clients reached\n");
        
        const char* response = "connect_failed";
        network_send_packet(network_get_bound_ip_socket(), strlen(response), response, net_addr);

        return;
    }

    clients[current_client_count].name = "Client"; // TODO: get actual client name
    clients[current_client_count].addr = net_addr;
    current_client_count++;

    const char* response = "connected";
    network_send_packet(network_get_bound_ip_socket(), strlen(response), response, net_addr);

    // char addr[INET_ADDRSTRLEN];
    // netadr_to_sockadr(net_addr, (struct sockaddr_in *)&client->ip);

    // printf("Client %s connected\n", addr);
}

static void server_disconnect_client(void) {
	// handle client disconnection
	client_t *client = malloc(sizeof(client_t));
	if (!client) {
		fprintf(stderr, "Failed to allocate memory for client\n");
		return;
	}
	client->name = "Client";

	// client will disconnect on their end,
	// no need to tell them we are removing them
}

void server_set_connected_clients_count(int count) {
    current_client_count = count;
    if (current_client_count > MAX_CLIENTS) {
        fprintf(stderr, "Too many clients connected: %d\n", current_client_count);
        current_client_count = MAX_CLIENTS; // cap at max
    }
}

unsigned int server_get_connected_clients_count(void) { return current_client_count; }

client_t *server_get_client_by_index(unsigned int index) {
    if(index >= current_client_count) {
        fprintf(stderr, "Index out of bounds: %d\n", index);
        return NULL;
    }
    
    return &clients[index];
}

static void server_status(struct sockaddr_in net_addr) {

    Wipeout__ServerInfo msg = WIPEOUT__SERVER_INFO__INIT;

    // TODO:
    // read from config or environment
    msg.name = "MY SERVER";
    msg.port = 8000;

    size_t len = wipeout__server_info__get_packed_size(&msg);
    uint8_t *buffer = malloc(len);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer for server info\n");
        return;
    }
    wipeout__server_info__pack(&msg, buffer);
    network_send_packet(network_get_bound_ip_socket(), len, buffer, net_addr);

    return;
}

/**
 * Parse messages received from a client,
 * and kick off any commands as appopriate
 */
static void server_parse_msg(msg_queue_item_t *item) {
    const char *cmd = item->command;
    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((struct sockaddr_in *)&item->dest_addr)->sin_addr,
              addr, sizeof(addr));
    char buf[100];

    // netadr_t net_addr;
    // sockadr_to_netadr((struct sockaddr_in *)&item->dest_addr, &net_addr);

    if (strcmp(cmd, "status") == 0) {
        // handle status
        server_status(item->dest_addr);
        return;
    } else if (strcmp(cmd, "connect") == 0) {
        // handle connect
        server_connect_client(item->dest_addr);
        return;
    } else if (strcmp(cmd, "disconnect") == 0) {
        // handle quit
        server_disconnect_client();
        sprintf(buf, "Client %s disconnected\n", addr);
    } else if (strcmp(cmd, "hello") == 0) {
        // handle hello (just echo back the client's address)
        sprintf(buf, "Hello from %s\n", addr);
    } else {
        sprintf(buf, "Unknown command: %s\n", cmd);
    }
    network_send_packet(network_get_bound_ip_socket(), strlen(buf), buf,
                        item->dest_addr);
}

void server_process_queue(void) {
	msg_queue_item_t item;
	while (network_get_msg_queue_item(&item)) {
		server_parse_msg(&item);
		network_popleft_msg_queue();
	}
}

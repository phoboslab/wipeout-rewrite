
#include "client.h"

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

thrd_t worker_threds[10];
int num_threads = 0;

typedef struct {
    const char *name;
    connect_state_t state;
} client_t;

/**
 * @brief Processes queued messages from clients
 */
static void server_response_thread(void) {}


static void server_connect_client(void) {
    // handle client connection
    client_t *client = malloc(sizeof(client_t));
    if (!client) {
        fprintf(stderr, "Failed to allocate memory for client\n");
        return;
    }
    client->name = "Client";
    client->state = CONNECTED;

	// TODO: tell client they are connected
}

static void server_disconnect_client(void) {
	// handle client disconnection
	client_t *client = malloc(sizeof(client_t));
	if (!client) {
		fprintf(stderr, "Failed to allocate memory for client\n");
		return;
	}
	client->name = "Client";
	client->state = DISCONNECTED;

	// TODO: client will disconnect on their end,
	// no need to tell them we are removing them
}

static void server_status(netadr_t *net_addr) {

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
    network_send_packet(network_get_bound_ip_socket(), len, buffer, *net_addr);

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

    netadr_t net_addr;
    sockadr_to_netadr((struct sockaddr_in *)&item->dest_addr, &net_addr);

    if (strcmp(cmd, "connect") == 0) {
        // handle connect
    } else if (strcmp(cmd, "disconnect") == 0) {
        // handle disconnect
    } else if (strcmp(cmd, "status") == 0) {
        // handle status
        server_status(&net_addr);

        return;
    } else if (strcmp(cmd, "quit") == 0) {
        // handle quit
        server_disconnect_client();
        sprintf(buf, "Client %s disconnected\n", addr);
    } else if (strcmp(cmd, "connect") == 0) {
        // handle connect
        server_connect_client();
        sprintf(buf, "Client %s connected\n", addr);
    } else if (strcmp(cmd, "hello") == 0) {
        // handle hello (just echo back the client's address)
        sprintf(buf, "Hello from %s\n", addr);
    } else {
        sprintf(buf, "Unknown command: %s\n", cmd);
    }
    network_send_packet(network_get_bound_ip_socket(), strlen(buf), buf,
                        net_addr);
}

void server_process_queue(void) {
	msg_queue_item_t item;
	while (network_get_msg_queue_item(&item)) {
		server_parse_msg(&item);
		network_popleft_msg_queue();
	}
}

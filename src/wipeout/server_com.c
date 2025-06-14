
#include "server_com.h"

#include "menu.h"
#include "network_wrapper.h"

#include <addr_conversions.h>
#include <network.h>
#include <ServerInfo.pb-c.h>

#if defined(WIN32)
#include <ws2ipdef.h>
#else
#include <arpa/inet.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <threads.h>

atomic_bool network_discovery_on = false;

thrd_t network_discovery_thread;
thrd_t network_discovery_response_thread;

static int DISCOVERY_TIMEOUT = 30; // seconds

static int sockfd = INVALID_SOCKET;

static server_info_t* servers = NULL; // dynamically allocated array of server_info_t
static unsigned int n_servers = 0;

static menu_page_t* server_menu_page = NULL; // menu for server discovery

void server_com_client_init(void) {
    // TODO
}

static void server_com_client_connect(menu_t*, int index) {
    server_info_t server = servers[index];
    printf("Connecting to server: %s\n", server.name);
}

static void server_com_update_servers() {
    // TODO:
    // is this function necessary?
    // we can't update or add a button after
    // startup, only change the text


    if(!server_menu_page) {
        printf("No server menu page set, cannot update servers.\n");
        return;
    }

    server_info_t server = servers[n_servers];
    char name[32];
    snprintf(name, sizeof(name), "%s", server.name);

    menu_page_add_button(server_menu_page, n_servers + 1, name, server_com_client_connect);
}

/**
 * @brief since we're connecting over UDP, need to have a thread running
 * separately to listen for discovery responses so we don't miss any
 */
static int server_com_discovery_response(void* arg) {
    (void)arg; // unused

    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    char buffer[1024];

    servers = realloc(servers, 0); // start with an empty list
    n_servers = 0;

    time_t start_time = time(NULL);

    while (true) {

        while(!network_discovery_on) {
            thrd_yield(); // wait until discovery is enabled
            start_time = time(NULL); // reset start time when we start listening
        }

        if(time(NULL) - start_time > DISCOVERY_TIMEOUT) {
            printf("No responses received in %d seconds, stopping discovery.\n", DISCOVERY_TIMEOUT);
            network_discovery_on = false; // stop listening
            continue;
        }

        ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer)-1, 0,
                               (struct sockaddr*)&from, &fromlen);

        // TODO: how do we need to handle recvfrom timeouts and errors?
        // if (len < 0) {
        //     if (errno == EWOULDBLOCK || errno == EAGAIN) {
        //         printf("Timeout reached.\n");
        //         break;
        //     } else {
        //         perror("recvfrom");
        //         break;
        //     }
        // }

        if (len > 0) {
            Wipeout__ServerInfo* msg = wipeout__server_info__unpack(NULL, len, (const uint8_t*)buffer);
            if(msg == NULL) {
                fprintf(stderr, "Failed to unpack server info message\n");
                continue;
            }

            buffer[len] = '\0';
            printf("Found server %s @ %s:%d\n", msg->name, inet_ntoa(from.sin_addr), msg->port);

            servers = realloc(servers, sizeof(server_info_t) * (n_servers + 1));
            servers[n_servers] = (server_info_t) {
                .name = msg->name
            };
            server_com_update_servers(); // update the menu with the new server
            n_servers++;
        }
    }

    return 1;
}

/**
 * @brief Run network discovery to find servers
 */
static int server_com_network_discovery(void* arg) {
    (void)arg;

#ifdef _WIN32
    if(!system_init_winsock()) {
        printf("Failed to initialize Winsock for network discovery.\n");
        return 0;
    }
#endif

    const char* message = "status";
    broadcast_list_t broadcasts = network_get_broadcast_addresses();

    for (size_t i = 0; i < broadcasts.count; ++i) {
        struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_port = htons(WIPEOUT_PORT),
            .sin_addr = broadcasts.list[i].broadcast,
        };

        wrap_sendto(sockfd, message, strlen(message), 0,
                    (struct sockaddr*)&addr, sizeof(addr));

        printf("[*] Broadcast packet sent. Waiting for replies...\n");
        // tell receiver thread to start listening
        network_discovery_on = true;
    }

    free(broadcasts.list);

    return 1;
}

void server_com_init_network_discovery(void) {

    sockfd = network_get_client_socket();
    if(sockfd == INVALID_SOCKET) {
        printf("unable to create socket to run network discovery\n");
        return;
    }

    char my_ip[INET_ADDRSTRLEN];
    network_get_my_ip(my_ip, INET_ADDRSTRLEN);

    if(!network_bind_socket(sockfd, my_ip, "8001")) {
        printf("unable to bind socket for network discovery\n");
        network_close_socket(&sockfd);
        return;
    }

    // immediately make us ready to get responses before we start broadcasting
    thrd_create(&network_discovery_response_thread, (thrd_start_t)server_com_discovery_response, NULL);
    thrd_detach(network_discovery_response_thread);

    thrd_create(&network_discovery_thread, (thrd_start_t)server_com_network_discovery, NULL);
    thrd_detach(network_discovery_thread);
}

void server_com_halt_network_discovery(void) {
    network_discovery_on = false;
    thrd_join(network_discovery_thread, NULL);
    thrd_join(network_discovery_response_thread, NULL);
}

server_info_t *server_com_get_servers(void) { 
    return servers; 
}

unsigned int server_com_get_n_servers(void) { 
    return n_servers; 
}

void server_com_set_menu_page(menu_page_t *page) {
    server_menu_page = page;
}

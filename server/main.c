
#include <client_com.h>

#include <msg.h>
#include <network.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#if defined(WIN32)
#else
#include <unistd.h>
#endif

typedef struct
{
    const char *name;
    int num_clients;
} server_t;

static server_t server;

static void server_init()
{
    server.name = "master blaster";
    server.num_clients = 0;

    client_com_init();
}

int main(int argc, char** argv)
{
    (void)argc; // unused
    (void)argv; // unused

    printf("welcome to the server!\n");

    server_init();

    int sockfd = network_get_client_socket();
    if(sockfd == INVALID_SOCKET) {
        printf("could not create socket, quitting\n");
        return 1;
    }
    if(!network_bind_socket(sockfd, "8000")) {
        printf("could not start server, quitting\n");
        return 1;
    }

    while (true)
    {
        if(network_sleep(100) <= 0) {
            // no network activity, continue
            continue;
        }

        network_get_packet();
        server_process_queue();
    }

    return 0;
}

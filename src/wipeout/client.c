
#include "client.h"

#include <network.h>

void client_init() {
    network_connect_ip("localhost");
}

void client_init_server_info(server_info_t *server, netadr_t *adr)
{
    server->adr = (*adr);
    server->clients = 0;
    server->host_name[0] = '\0';
}

void client_get_server_info() {
    //network_send_command("server_info");
}

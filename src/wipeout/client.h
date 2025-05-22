
#pragma once

#include <network_types.h>

typedef struct {
    netadr_t    adr;
    char host_name[256];
    int clients;
} server_info_t;


void client_init();

void client_init_server_info(server_info_t* server, netadr_t* adr);

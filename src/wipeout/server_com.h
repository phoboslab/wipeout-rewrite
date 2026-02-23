
#pragma once

#include "menu.h"

/*
client communication to server(s),
including server discovery
*/

typedef struct server_info_t server_info_t;

void server_com_set_menu_page(menu_page_t *page);

void server_com_client_init(void);

void server_com_init_network_discovery(void);

void server_com_halt_network_discovery(void);

server_info_t* server_com_get_servers(void);

unsigned int server_com_get_n_servers(void);
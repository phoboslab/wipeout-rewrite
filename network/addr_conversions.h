
#pragma once

#include "network_types.h"

struct sockaddr;
struct sockaddr_in;

// translate a network address (like localhost) or ip to a soecket address
void string_to_socket_addr(const char *str_addr, struct sockaddr *sadr);

void string_to_addr(const char* str, netadr_t* addr);

const char *addr_to_string(netadr_t addr);

void netadr_to_sockadr(netadr_t *addr, struct sockaddr_in *s);

void sockadr_to_netadr(struct sockaddr_in *s, netadr_t *addr);


#ifndef CLIENT_H
#define CLIENT_H

#include <network_types.h>

void server_connect_client(netadr_t from);

/**
 * Parse messages received from a client,
 * and kick off any commands as appopriate
 */
void server_parse_msg(msg_t *msg);

#endif
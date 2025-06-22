
#include "addr_conversions.h"

#if defined(WIN32)
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include <stdio.h>
#include <string.h>

void string_to_socket_addr(const char *str_addr, struct sockaddr *sadr)
{
    struct hostent *h;

    memset(sadr, 0, sizeof(*sadr));
    ((struct sockaddr_in *)sadr)->sin_family = AF_INET;

    ((struct sockaddr_in *)sadr)->sin_port = 0;

    if (str_addr[0] >= '0' && str_addr[0] <= '9')
    {
        // it's an ip4 address
        inet_pton(AF_INET, str_addr, &((struct sockaddr_in *)sadr)->sin_addr);
    }
    else
    {
        // it's a string address
        if (!(h = gethostbyname(str_addr)))
            return;
        *(int *)&((struct sockaddr_in *)sadr)->sin_addr = *(int *)h->h_addr_list[0];
    }

    return;
}

void string_to_addr(const char* str, netadr_t* addr) {
    struct sockaddr_in sadr;

    string_to_socket_addr(str, (struct sockaddr*)&sadr);

    sockadr_to_netadr(&sadr, addr);
}

const char *addr_to_string(netadr_t addr)
{
    static char str[64];

    snprintf(str, sizeof(str), "%i.%i.%i.%i:%hu", addr.ip[0], addr.ip[1], addr.ip[2], addr.ip[3], addr.port);

    return str;
}

void netadr_to_sockadr(netadr_t *addr, struct sockaddr_in *s)
{

    memset(s, 0, sizeof(*s));

    s->sin_family = AF_INET;
    *(int *)&s->sin_addr = *(int *)&addr->ip;
    s->sin_port = addr->port;
}

void sockadr_to_netadr(struct sockaddr_in *s, netadr_t *addr)
{
    *(int *)&addr->ip = *(int *)&s->sin_addr;
    addr->port = s->sin_port;
}
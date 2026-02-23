
#pragma once

#include <stdbool.h>
#if defined(WIN32)
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

typedef unsigned char byte;

#define MAX_STRING_CHARS 1024
#define MAX_MSGLEN 16

typedef struct {
    byte ip[4];
    // N.B. needs to be big-endian
    unsigned short port;
} netadr_t;

typedef struct {
    int cursize;
    int readcount;
    int bit;
    int maxsize;
    bool oob;
    byte* data;
} msg_t;

typedef enum {
    CLIENT,
    SERVER,
} network_role_t;

typedef struct {
    struct in_addr broadcast;
} broadcast_addr_t;

typedef struct {
    broadcast_addr_t* list;
    size_t count;
} broadcast_list_t;
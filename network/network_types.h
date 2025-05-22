
#pragma once

#include <stdbool.h>

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
    SERVER
} netsrc_t;

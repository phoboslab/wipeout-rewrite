
#include "client.h"

#include <msg.h>
#include <network.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PACKET_HDR_SIZE 4

typedef struct
{
    const char *name;
    int num_clients;
} server_t;

server_t server;

// command callbacks
static void server_status()
{
}

static void server_anonymous_packet(netadr_t from, msg_t *msg)
{

    if (strncmp("connect", &msg->data[PACKET_HDR_SIZE], strlen("connect") != 0))
    {
        // a more complex message
    }

    char *s = msg_read_string_line(msg);
    char *tokens = strtok(s, " ");

    if (strcmp(tokens, "getstatus") == 0)
    {
        server_status();
    }
    else if (strcmp(tokens, "connect") == 0)
    {
        server_connect_client(from);
    }
    else if (strcmp(tokens, "command") == 0)
    {
        // perform a command given the message
    }
    else
    {
        printf("this packet is bad...\n");
    }
}

static void packet_event(netadr_t from, msg_t *msg)
{

    // is it from an anonymous client (i.e. one not connected to us)
    // starts with 0xffff_ffff
    if (msg->cursize >= PACKET_HDR_SIZE && *(int *)msg->data == -1)
    {
        server_anonymous_packet(from, msg);
        return;
    }

    // the header is different lengths for reliable and unreliable messages
    int headerBytes = msg->readcount;

    // track the last message received so it can be returned in
    // client messages, allowing the server to detect a dropped
    // gamestate
    // clc.serverMessageSequence = LittleLong( *(int *)msg->data );

    // clc.lastPacketTime = cls.realtime;
    server_parse_msg(msg);
}

static void server_init()
{
    server.name = "master blaster";
    server.num_clients = 0;
}

int main(int, char **)
{

    printf("welcome to the server!\n");

    server_init();

    bool should_quit = false;
    netadr_t evFrom;
    msg_t buf;

    network_bind_ip();

    while (!should_quit)
    {
        int ret = network_sleep(100);

        if(ret == 0) {
            continue;
        }

        // TODO: how do we know _IF_ we received a packet?
        while (network_get_packet())
        {
            packet_event(evFrom, &buf);
        }
    }

    return 0;
}
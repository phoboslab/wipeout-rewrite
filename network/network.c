
#include "network.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <Windows.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <ServerInfo.pb-c.h>

#define INVALID_SOCKET -1
#define SERVER_PORT "8000"

#endif

#if defined(WIN32)
static WSADATA winsockdata;
#endif

int ip_socket;

const char *network_get_last_error()
{
#if defined(WIN32)
    DWORD code = WSAGetLastError();

    char *errorMsg = NULL;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&errorMsg,
        0,
        NULL);

    return errorMsg;

#else
    return strerror(errno);
#endif
}


// perhaps a platform specific interface to send a packet? it's really sent here
static void system_send_packet(int length, const void *data, netadr_t dest_net)
{

    if (!ip_socket)
    {
        printf("ip connection hasn't been established...");
        return;
    }

    int net_socket = ip_socket;
    struct sockaddr_in dest_addr;

    netadr_to_sockadr(&dest_net, &dest_addr);

    int ret = sendto(net_socket, data, length, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

    if (ret == -1)
    {
        printf("unable to send packet to %s: %s\n", addr_to_string(dest_net), network_get_last_error());
    }
}

static bool system_get_packet(netadr_t *net_from, msg_t *net_msg)
{
    struct sockaddr_in from;

    if (!ip_socket)
        return false;

    int fromlen = sizeof(from);
    int ret = recvfrom(ip_socket, net_msg->data, net_msg->maxsize, 0, (struct sockaddr *)&from, &fromlen);

    sockadr_to_netadr(&from, net_from);
    net_msg->readcount = 0;

    if (ret == -1)
    {
        int err = errno;

        if (err == EWOULDBLOCK || err == ECONNREFUSED) {
            return false;
        }
        
        printf("%s from %s\n", network_get_last_error(), addr_to_string(*net_from));
    }

    if (ret == net_msg->maxsize)
    {
        printf("oversize packet from %s\n", addr_to_string(*net_from));
        return false;
    }

    net_msg->cursize = ret;
    return true;
}

// attempt to open network connection
int network_ip_socket(char *ip_addr, int port)
{

#if defined(WIN32)
    SOCKET new_socket;
#else
    int new_socket;
#endif

    struct sockaddr_in address;

    string_to_socket_addr(ip_addr, (struct sockaddr *)&address);

    address.sin_port = htons((short)port);
    address.sin_family = AF_INET;

    bool _true = true;
    int i = 1;

    if ((new_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        printf("unable to open socket: %s\n", network_get_last_error());
        return 0;
    }

#if defined(WIN32)
    if (ioctlsocket(new_socket, FIONBIO, &_true) == -1)
    {
#else
    if (ioctl(new_socket, FIONBIO, &_true) == -1)
    {
#endif
        printf("can't make socket non-blocking: %s\n", network_get_last_error());
        return 0;
    }

    if (setsockopt(new_socket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) == -1)
    {
        printf("can't make socket broadcastable: %s\n", network_get_last_error());
        return 0;
    }

    if (bind(new_socket, (void *)&address, sizeof(address)) == -1)
    {
        printf("couldn't bind address and port: %s\n", network_get_last_error());
#if defined(WIN32)
        closesocket(new_socket);
#else
        close(new_socket);
#endif
        return 0;
    }

    return new_socket;
}

bool network_has_ip_socket(void)
{
    return ip_socket;
}

void network_bind_ip(void)
{

#if defined(WIN32)
    if ((WSAStartup(MAKEWORD(1, 1), &winsockdata)))
    {
        printf("unable to init windows socket: %s\n", network_get_last_error());
        return;
    }
#endif

    char *address = "localhost";
    int port = 8000;

    // make several attempts to grab an open port
    for (int i = 0; i < 10; i++)
    {
        port += i;
        ip_socket = network_ip_socket(address, port);

        if (ip_socket)
        {
            printf("established connection at %s:%d\n", address, port);
            return;
        }
    }
    perror("could not establish network connection... quitting.\n");
}

void network_connect_ip(const char* addr)
{
    struct addrinfo hints;
    struct addrinfo* servinfo;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int ret;

    if((ret = getaddrinfo(addr, SERVER_PORT, &hints, &servinfo)) != 0) {
        printf("getaddrinfo error: %s\n", network_get_last_error());
        return;
    }

    // loop through all the results and connect to the first we can
    for(struct addrinfo* p = servinfo; p != NULL; p = p->ai_next) {
        if ((ip_socket = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(ip_socket, p->ai_addr, p->ai_addrlen) == -1) {
            //close(ip_socket);
            perror("client: connect");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);
}

void network_close_connection()
{
    // TODO: how to confirm it's an open socket?
    if(ip_socket > 0) {
#if defined(WIN32)
        closesocket(ip_socket);
#else
        close(ip_socket);
#endif
    }
}

void network_out_of_band_print(netsrc_t sock, netadr_t adr, const char *format, ...)
{
    va_list argptr;
    char str[1024];

    // header
    str[0] = -1;
    str[1] = -1;
    str[2] = -1;
    str[3] = -1;

    va_start(argptr, format);
    vsprintf(str + 4, format, argptr);
    va_end(argptr);

    network_send_packet(sock, strlen(str), str, adr);
}

bool network_get_packet()
{
    int MAXBUFLEN = 100;
    char buf[MAXBUFLEN];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    int numbytes = 0;

    char s[INET_ADDRSTRLEN];

    if ((numbytes = recvfrom(ip_socket, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    struct sockaddr* addr = &((struct sockaddr_in*)&their_addr)->sin_addr;

    printf("listener: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            addr,
            s, sizeof s));
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buf);
    return true;
}

void network_send_packet(netsrc_t sock, int length, const void *data, netadr_t dest_net)
{
    system_send_packet(length, data, dest_net);
}

// there needs to be enough loopback messages to hold a complete
// gamestate of maximum size
#define MAX_LOOPBACK 16
#define MAX_PACKETLEN 1024

typedef struct
{
    byte data[MAX_PACKETLEN];
    int datalen;
} loopmsg_t;

typedef struct
{
    loopmsg_t msgs[MAX_LOOPBACK];
    int get, send;
} loopback_t;

// one for client, one for server
loopback_t loopbacks[2];

bool network_get_loop_packet(netsrc_t sock, netadr_t *net_from, msg_t *net_message)
{
    loopback_t *loop = &loopbacks[sock];

    if (loop->send - loop->get > MAX_LOOPBACK)
    {
        loop->get = loop->send - MAX_LOOPBACK;
    }

    if (loop->get >= loop->send) {
        return false;
    }

    int i = loop->get & (MAX_LOOPBACK - 1);
    loop->get++;

    memcpy(net_message->data, loop->msgs[i].data, loop->msgs[i].datalen);
    net_message->cursize = loop->msgs[i].datalen;
    memset(net_from, 0, sizeof(*net_from));

    return true;
}

void network_send_loop_packet(netsrc_t sock, int length, const void *data, netadr_t to)
{
    loopback_t *loop = &loopbacks[sock ^ 1];

    int i = loop->send & (MAX_LOOPBACK - 1);
    loop->send++;

    memcpy(loop->msgs[i].data, data, length);
    loop->msgs[i].datalen = length;
}

void network_send_command(const char *command, netadr_t dest)
{
    if(strcmp(command, "server_info") == 0) {
        Wipeout__ServerInfo* msg;
        wipeout__server_info__init(msg);
        network_send_packet(ip_socket, strlen(command), "server_info", dest);

    }
}

void network_process_command(const char* command) 
{
    if(strcmp(command, "server_info") == 0) {
        Wipeout__ServerInfo* msg;
        wipeout__server_info__init(msg);
        msg->name = "my server";
        msg->port = 8000;

        unsigned char* out = (unsigned char*)malloc(wipeout__server_info__get_packed_size(msg));
        if(out) {
            wipeout__server_info__pack(msg, out);
        }
    }
}

int network_sleep(int msec)
{
    struct timeval timeout;
    fd_set fdset;

    if (!ip_socket) {
        return 0;
    }

    FD_ZERO(&fdset);
    FD_SET(ip_socket, &fdset); // network socket
    timeout.tv_sec = msec / 1000;
    timeout.tv_usec = (msec % 1000) * 1000;
    return select(ip_socket + 1, &fdset, NULL, NULL, &timeout);
}

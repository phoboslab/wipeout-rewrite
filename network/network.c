
#include "network.h"
#include "network_wrapper.h"

#include "addr_conversions.h"
#include "msg.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(WIN32)
#include <iphlpapi.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ifaddrs.h>

#endif

#include <ServerInfo.pb-c.h>

#if defined(WIN32)
static WSADATA winsockdata;
static bool winsock_initialized = false;
#endif

static msg_queue_item_t msg_queue[10];
static int msg_queue_size = 0;

static int client_sockfd = INVALID_SOCKET;

static const char *network_get_last_error(void)
{
#if defined(WIN32)
    int code = WSAGetLastError();

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

static void system_send_packet(int sockfd, int length, const void *data, struct sockaddr_in dest_net)
{

    if(sockfd == INVALID_SOCKET)
    {
        printf("unable to get socket for sending packet: %s\n", network_get_last_error());
        return;
    }


    int ret = wrap_sendto(sockfd, data, length, 0, (struct sockaddr *)&dest_net, sizeof(dest_net));

    if (ret == -1)
    {
       // printf("unable to send packet to %s: %s\n", addr_to_string(dest_net), network_get_last_error());
    }
}


#if defined(WIN32)
bool system_init_winsock(void) {
    if (!winsock_initialized) {
        if ((WSAStartup(MAKEWORD(2, 2), &winsockdata) != 0)) {
            printf("unable to init windows socket: %s\n",
                   network_get_last_error());
            return false;
        }
        winsock_initialized = true;
    }

    return true;
}

void system_cleanup_winsock(void) {
    if (winsock_initialized) {
        WSACleanup();
        winsock_initialized = false;
    }
}
#endif

int network_get_client_socket(void) {

#if defined(WIN32)
    if(!system_init_winsock()) {
        return INVALID_SOCKET;
    }
#endif


#if defined(WIN32)
    SOCKET new_socket = INVALID_SOCKET;
#else
    int new_socket = INVALID_SOCKET;
#endif

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, "8000", &hints, &res);

    if ((new_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) ==
        INVALID_SOCKET) {
        printf("unable to open socket: %s\n", network_get_last_error());
        return INVALID_SOCKET;
    }

#if defined(WIN32)
    unsigned long _true = 1;
    if (ioctlsocket(new_socket, FIONBIO, &_true) == -1) {
#else
    bool _true = true;
    if (ioctl(new_socket, FIONBIO, &_true) == -1) {
#endif
        printf("can't make socket non-blocking: %s\n",
               network_get_last_error());

        network_close_socket(&new_socket);
        return INVALID_SOCKET;
    }

    int i = 1;
    if (setsockopt(new_socket, SOL_SOCKET, SO_BROADCAST, (char *)&i,
                   sizeof(i)) == -1) {
        printf("can't make socket broadcastable: %s\n",
               network_get_last_error());
        network_close_socket(&new_socket);
        return INVALID_SOCKET;
    }

    // Set receive timeout
    // TODO: can this fail?
    struct timeval timeout;
    timeout.tv_sec = CLIENT_SOCKET_TIMEOUT / 1000; // convert milliseconds to seconds
    timeout.tv_usec = 0;
    setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    return new_socket;
}

bool network_bind_socket(int sockfd, char* port)
{
    if(client_sockfd != INVALID_SOCKET)
    {
        printf("socket already bound, cannot bind again\n");
        return true;
    }

    if(sockfd == INVALID_SOCKET)
    {
        printf("could not create socket: %s\n", network_get_last_error());
        return false;
    }

    struct addrinfo hints;
    struct addrinfo* res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, &hints, &res);

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        printf("couldn't bind address and port: %s\n", network_get_last_error());
        return false;
    }

    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &(addr->sin_addr), ipstr, sizeof(ipstr));

    printf("established connection at %s:%s\n", ipstr, port);
    network_set_bound_ip_socket(sockfd);

    return true;
}

bool network_has_bound_ip_socket(void)
{
    return client_sockfd != INVALID_SOCKET;
}

void network_set_bound_ip_socket(int sockfd)
{
    client_sockfd = sockfd;
}

int network_get_bound_ip_socket(void) { 
    return client_sockfd; 
}

void network_close_socket(int* sockfd) {
    if(*sockfd == INVALID_SOCKET) {
        return;
    }
#if defined(WIN32)
    closesocket(*sockfd);
#else
    close(*sockfd);
#endif

    *sockfd = INVALID_SOCKET;
}

static void network_add_msg_queue_item(const char *buf, const struct sockaddr_in* their_addr) {
    msg_queue_item_t *item = (msg_queue_item_t *)malloc(sizeof(msg_queue_item_t));
    if (!item) {
        perror("Failed to allocate memory for message queue item");
        return;
    }

    item->command = strdup(buf);
    if (!item->command) {
        perror("Failed to allocate memory for command string");
        free(item);
        return;
    }

    memcpy(&item->dest_addr, their_addr, sizeof(struct sockaddr_in));
    msg_queue[msg_queue_size++] = *item;
}

int network_get_msg_queue_size() {
    return msg_queue_size;
}

void network_clear_msg_queue() {
    for (int i = 0; i < msg_queue_size; i++) {
        free(msg_queue[i].command);
    }
    msg_queue_size = 0;
}

void network_popleft_msg_queue() {
    if (msg_queue_size > 0) {
        free(msg_queue[0].command);
        memmove(&msg_queue[0], &msg_queue[1], (msg_queue_size - 1) * sizeof(msg_queue_item_t));
        msg_queue_size--;
    }
}

bool network_get_msg_queue_item(msg_queue_item_t *item) {
    if (msg_queue_size == 0) {
        return false;
    }

    *item = msg_queue[0];

    return true;
}


bool network_get_packet()
{
    int MAXBUFLEN = 100;
    char buf[MAXBUFLEN];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    int numbytes = 0;

    if ((numbytes = wrap_recvfrom(client_sockfd, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now, not a fatal error
            return false;
        } else {
            perror("recvfrom");
            exit(1);
        }
    }

    buf[numbytes] = '\0';
    network_add_msg_queue_item(buf, (struct sockaddr_in*)&their_addr);

    return true;
}

void network_send_packet(int sockfd, int length, const void *data, struct sockaddr_in dest_net)
{
    system_send_packet(sockfd, length, data, dest_net);
}

int network_sleep(int msec)
{
    struct timeval timeout;
    fd_set fdset;

    if(!network_has_bound_ip_socket()) {
        printf("no socket bound, nothing to wait for\n");
        return 0; // no socket bound, nothing to wait for
    }

    FD_ZERO(&fdset);
    FD_SET(client_sockfd, &fdset); // network socket
    timeout.tv_sec = msec / 1000;
    timeout.tv_usec = (msec % 1000) * 1000;
    return select(client_sockfd + 1, &fdset, NULL, NULL, &timeout);
}

void network_get_my_ip(char *subnet, size_t len) {

    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53); // DNS
    inet_pton(AF_INET, "8.8.8.8", &dest.sin_addr);

    int sockfd = network_get_client_socket();
    if(sockfd == INVALID_SOCKET) {
        perror("could not get ip address, quitting\n");
        return;
    }

    connect(sockfd, (struct sockaddr *)&dest, sizeof(dest));

    struct sockaddr_in local;
    len = sizeof(local);
    getsockname(sockfd, (struct sockaddr *)&local, &len);

    strncpy(subnet, inet_ntoa(local.sin_addr), len);

    network_close_socket(&sockfd);
}

/**
 * @brief Get broadcast addresses for LAN
 * 
 * @return broadcast_list_t 
 */
broadcast_list_t network_get_broadcast_addresses(void) {
    broadcast_list_t result = {0};
    size_t capacity = 8;

#if defined(WIN32)
    DWORD size = 0;
    GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &size);
    IP_ADAPTER_ADDRESSES* adapters = (IP_ADAPTER_ADDRESSES*)malloc(size);
    if (!adapters || GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, adapters, &size) != NO_ERROR) {
        free(adapters);
        return result;
    }

    result.list = (broadcast_addr_t*)malloc(sizeof(broadcast_addr_t) * capacity);

    for (IP_ADAPTER_ADDRESSES* adapter = adapters; adapter; adapter = adapter->Next) {
        if (adapter->IfType != IF_TYPE_ETHERNET_CSMACD && adapter->IfType != IF_TYPE_IEEE80211)
            continue;

        for (IP_ADAPTER_UNICAST_ADDRESS* ua = adapter->FirstUnicastAddress; ua; ua = ua->Next) {
            SOCKADDR_IN* sa = (SOCKADDR_IN*)ua->Address.lpSockaddr;
            uint32_t ip = ntohl(sa->sin_addr.S_un.S_addr);
            uint32_t mask = (0xFFFFFFFF << (32 - ua->OnLinkPrefixLength)) & 0xFFFFFFFF;
            uint32_t bcast = (ip & mask) | (~mask);
            
            if (result.count >= capacity) {
                capacity *= 2;
                result.list = realloc(result.list, sizeof(broadcast_addr_t) * capacity);
            }

            result.list[result.count++].broadcast.s_addr = htonl(bcast);
        }
    }

    free(adapters);
#else
    struct ifaddrs* ifaddr = NULL;
    if (getifaddrs(&ifaddr) == -1) {
        return result;
    }

    result.list = (broadcast_addr_t*)malloc(sizeof(broadcast_addr_t) * capacity);

    for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;

        if ((ifa->ifa_flags & IFF_LOOPBACK) || !(ifa->ifa_flags & IFF_BROADCAST))
            continue;

        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);

        if (ioctl(network_get_client_socket(), SIOCGIFBRDADDR, &ifr) < 0)
            continue;

        struct sockaddr_in* baddr = (struct sockaddr_in*)&ifr.ifr_broadaddr;
        if (result.count >= capacity) {
            capacity *= 2;
            result.list = realloc(result.list, sizeof(broadcast_addr_t) * capacity);
        }

        result.list[result.count++].broadcast = baddr->sin_addr;
    }

    freeifaddrs(ifaddr);
#endif

    return result;
}

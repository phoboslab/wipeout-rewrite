#pragma once

#if defined(_WIN32)
#include <WinSock2.h>
#include "win_defines.h"
#else
#include <sys/socket.h>
#endif

ssize_t wrap_recvfrom(int sockfd, void *buf, size_t len, int flags,
                    struct sockaddr *src_addr, socklen_t *addrlen);

ssize_t wrap_sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

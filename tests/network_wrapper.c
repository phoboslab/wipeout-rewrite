
#include <network_wrapper.h>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

#if defined(WIN32)
#include <WinSock2.h>
#include <win_defines.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

// protobufs
#include <ServerInfo.pb-c.h>

ssize_t wrap_recvfrom(int sockfd, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, socklen_t *addrlen) {
    check_expected(sockfd);
    check_expected(len);
    check_expected(flags);

    // Optionally check or set addr and addrlen if relevant
    if (src_addr != NULL && addrlen != NULL) {
        memcpy(src_addr, mock_ptr_type(struct sockaddr *), sizeof(struct sockaddr));
        *addrlen = mock_type(socklen_t);
    }

    // Optionally simulate data in the buffer
    const char *data = mock_ptr_type(char *);
    size_t data_len = strlen(data); // or however long you need
    memcpy(buf, data, data_len < len ? data_len : len);

    return (ssize_t)mock_type(int); // number of bytes to return
}

ssize_t wrap_sendto(int sockfd, const void *buf, size_t len, int flags,
                    const struct sockaddr *dest_addr, socklen_t addrlen) {
    check_expected(sockfd);
    check_expected(len);
    check_expected(flags);
    check_expected(buf);

    // Get message type for this test
    const char *msg_type = mock_type(const char *);

    if (strcmp(msg_type, "STATUS") == 0) {
        Wipeout__ServerInfo *msg = wipeout__server_info__unpack(NULL, len, buf);
        assert_non_null(msg);
        const char *expected_name = mock_type(const char *);
        int expected_port = mock_type(int);

        assert_string_equal(msg->name, expected_name);
        assert_int_equal(msg->port, expected_port);
        wipeout__server_info__free_unpacked(msg, NULL);
    } else if (strcmp(msg_type, "STRING") == 0) {
        // const char *expected = mock_type(const char *);
        // assert_memory_equal(buf, expected, len);
    } else {
        fail_msg("Unknown message type in wrap_sendto");
    }

    // Optionally check or set dest_addr and addrlen if relevant
    if (dest_addr != NULL && addrlen > 0) {
        // TODO
        //check_expected(dest_addr->sa_data);
        check_expected(dest_addr->sa_family);
    }

    return (ssize_t)mock_type(int); // number of bytes sent
}
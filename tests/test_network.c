

#include <network.h>
#include <network_wrapper.h>

#include "utils.h"

#include <errno.h>
#if defined(WIN32)
#include <ws2ipdef.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

void network_close_socket_sets_socket_invalid(void** state) {
    (void)state; // unused

    int sockfd = 3; // Example socket descriptor
    network_set_bound_ip_socket(sockfd);

    // Ensure the socket is set
    assert_int_equal(network_get_bound_ip_socket(), sockfd);

    // Close the socket
    network_close_socket(&sockfd);

    // Check that the socket is now INVALID_SOCKET
    assert_int_equal(sockfd, INVALID_SOCKET);

    // cleanup
    network_test_cleanup();
}

void test_network_get_packet(void** state) {
    (void)state; // unused

    const char *mock_data = "Hello, World!";
    struct sockaddr_in mock_addr = {0};
    socklen_t mock_addr_len = sizeof(mock_addr);

    expect_value(wrap_recvfrom, sockfd, 3);
    expect_value(wrap_recvfrom, len, 99);
    expect_value(wrap_recvfrom, flags, 0);
    will_return(wrap_recvfrom, &mock_addr);        // for src_addr
    will_return(wrap_recvfrom, mock_addr_len);     // for addrlen
    will_return(wrap_recvfrom, mock_data);         // for buf content
    will_return(wrap_recvfrom, strlen(mock_data)); // return value

    network_set_bound_ip_socket(3); // Set the socket descriptor

    bool result = network_get_packet();

    // Check the result
    assert_true(result);

    // check message queue, should have one item
    int queue_size = network_get_msg_queue_size();
    assert_int_equal(queue_size, 1);

    // cleanup
    network_test_cleanup();
}

void test_network_get_packet_no_data(void**) {
    network_clear_msg_queue();

    const char *mock_data = "Hello, World!";
    struct sockaddr_in mock_addr = {0};
    socklen_t mock_addr_len = sizeof(mock_addr);

    expect_value(wrap_recvfrom, sockfd, 3);
    expect_value(wrap_recvfrom, len, 99);
    expect_value(wrap_recvfrom, flags, 0);
    will_return(wrap_recvfrom, &mock_addr);        // for src_addr
    will_return(wrap_recvfrom, mock_addr_len);     // for addrlen
    will_return(wrap_recvfrom, mock_data);         // for buf content
    will_return(wrap_recvfrom, -1); // return value

    network_set_bound_ip_socket(3); // Set the socket descriptor
    errno = EAGAIN; // Simulate no data available
    bool result = network_get_packet();
    assert_false(result);

    // check message queue, should still be empty
    int queue_size = network_get_msg_queue_size();
    assert_int_equal(queue_size, 0);

    // cleanup
    network_test_cleanup();
}

void test_network_get_local_subnet(void** state) {
    (void)state; // unused

    char my_ip[INET_ADDRSTRLEN];
    network_get_my_ip(my_ip, INET_ADDRSTRLEN);

    assert_non_null(my_ip);
    assert_string_not_equal(my_ip, "127.0.0.1");

    // this utility shouldn't change or set the bound socket
    // for this client
    int sockfd = network_get_bound_ip_socket();
    assert_int_equal(sockfd, INVALID_SOCKET);

    // cleanup
    network_test_cleanup();
}

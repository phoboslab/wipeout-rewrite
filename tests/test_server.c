#include <network.h>
#include <client.h>

#include "utils.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

#if defined(WIN32)
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include <ServerInfo.pb-c.h>


void empties_queue_after_process(void**) {

    network_clear_msg_queue();
    const char *mock_data = "hello";
    struct sockaddr_in mock_addr = {0};
    socklen_t mock_addr_len = sizeof(mock_addr);

    expect_value(wrap_recvfrom, sockfd, 3);
    expect_value(wrap_recvfrom, len, 99);
    expect_value(wrap_recvfrom, flags, 0);
    will_return(wrap_recvfrom, &mock_addr);
    will_return(wrap_recvfrom, mock_addr_len);
    will_return(wrap_recvfrom, mock_data);
    will_return(wrap_recvfrom, strlen(mock_data)); // return value


    const char* returned_data = "Hello from 0.0.0.0\n";

    expect_value(wrap_sendto, sockfd, 3);
    expect_string(wrap_sendto, buf, returned_data);
    expect_value(wrap_sendto, len, strlen(returned_data));
    expect_value(wrap_sendto, flags, 0);
    expect_value(wrap_sendto, dest_addr->sa_family, AF_INET);
    // TODO
    // expect_value(wrap_sendto, dest_addr->sa_data, &mock_addr);        // for dest_addr

    will_return(wrap_sendto, "STRING");  // tag for wrap_sendto to know how to interpret buf
    will_return(wrap_sendto, strlen(returned_data)); // return value

    network_set_bound_ip_socket(3);
    network_get_packet();

    int queue_size = network_get_msg_queue_size();
    assert_int_equal(queue_size, 1);
    
    // should run a step to check if we have work to do
    server_process_queue();

    queue_size = network_get_msg_queue_size();
    assert_int_equal(queue_size, 0);

    // cleanup
    network_test_cleanup();
}

void unknown_message_echo(void**) {

    network_clear_msg_queue();
    const char *mock_data = "unknown";
    struct sockaddr_in mock_addr = {0};
    socklen_t mock_addr_len = sizeof(mock_addr);

    expect_value(wrap_recvfrom, sockfd, 3);
    expect_value(wrap_recvfrom, len, 99);
    expect_value(wrap_recvfrom, flags, 0);
    will_return(wrap_recvfrom, &mock_addr);
    will_return(wrap_recvfrom, mock_addr_len);
    will_return(wrap_recvfrom, mock_data);
    will_return(wrap_recvfrom, strlen(mock_data)); // return value

    const char* returned_data = "Unknown command: unknown\n";

    expect_value(wrap_sendto, sockfd, 3);
    expect_string(wrap_sendto, buf, returned_data);
    expect_value(wrap_sendto, len, strlen(returned_data));
    expect_value(wrap_sendto, flags, 0);
    expect_value(wrap_sendto, dest_addr->sa_family, AF_INET);
    // TODO
    // expect_value(wrap_sendto, dest_addr->sa_data, &mock_addr);        // for dest_addr

    will_return(wrap_sendto, "STRING");  // tag for wrap_sendto to know how to interpret buf
    will_return(wrap_sendto, strlen(returned_data)); // return value

    network_set_bound_ip_socket(3);
    network_get_packet();

    int queue_size = network_get_msg_queue_size();
    assert_int_equal(queue_size, 1);

    server_process_queue();

    // cleanup
    network_test_cleanup();
}

void server_status_query(void**) {

    network_clear_msg_queue();
    const char *mock_data = "status";
    struct sockaddr_in mock_addr = {0};
    socklen_t mock_addr_len = sizeof(mock_addr);

    expect_value(wrap_recvfrom, sockfd, 3);
    expect_value(wrap_recvfrom, len, 99);
    expect_value(wrap_recvfrom, flags, 0);
    will_return(wrap_recvfrom, &mock_addr);
    will_return(wrap_recvfrom, mock_addr_len);
    will_return(wrap_recvfrom, mock_data);
    will_return(wrap_recvfrom, strlen(mock_data)); // return value

    // response to status query
    expect_value(wrap_sendto, sockfd, 3);
    expect_not_value(wrap_sendto, buf, NULL);
    expect_any(wrap_sendto, len);
    expect_value(wrap_sendto, flags, 0);
    expect_value(wrap_sendto, dest_addr->sa_family, AF_INET);

    will_return(wrap_sendto, "STATUS");  // tag for wrap_sendto to know how to interpret buf
    will_return(wrap_sendto, "MY SERVER");
    will_return(wrap_sendto, 8000);
    will_return(wrap_sendto, 0); // return value

    network_set_bound_ip_socket(3);
    network_get_packet();

    server_process_queue();

    // cleanup
    network_test_cleanup();
}
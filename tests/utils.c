
#include "utils.h"

#include <network.h>

void network_test_cleanup(void) {
    // Reset the network state after each test
    network_clear_msg_queue();
    network_set_bound_ip_socket(INVALID_SOCKET);
}
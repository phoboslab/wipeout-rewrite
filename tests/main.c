
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

extern void test_network_get_packet(void **state);
extern void test_network_get_packet_no_data(void **state);
extern void test_network_get_local_subnet(void **state);
extern void network_close_socket_sets_socket_invalid(void **state);


extern void empties_queue_after_process(void **state);
extern void unknown_message_echo(void **state);
extern void server_status_query(void **state);

int main(void) {
    const struct CMUnitTest tests[] = {
        // network library tests
        cmocka_unit_test(test_network_get_packet),
        cmocka_unit_test(test_network_get_packet_no_data),
        cmocka_unit_test(test_network_get_local_subnet),
        cmocka_unit_test(network_close_socket_sets_socket_invalid),

        // dedicated server tests
        cmocka_unit_test(empties_queue_after_process),
        cmocka_unit_test(unknown_message_echo),
        cmocka_unit_test(server_status_query),
    };
 
    return cmocka_run_group_tests(tests, NULL, NULL);
}
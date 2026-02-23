

add_executable(tests
    main.c
    test_network.c
    test_server.c

# mocks and test utilities
    network_wrapper.c
    utils.c
)

target_link_libraries(tests PRIVATE
    cmocka::cmocka
    network
    serverlib
)
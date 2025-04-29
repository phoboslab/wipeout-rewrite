
add_executable(
    server
    main.c
    client.c
)

target_link_libraries(
    server PRIVATE
    network
)
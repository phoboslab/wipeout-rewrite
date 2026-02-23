
add_subdirectory(serverlib)

add_executable(
    server
    main.c
)

target_link_libraries(
    server PRIVATE
    serverlib
    network
)
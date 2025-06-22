
add_library(serverlib STATIC
    client_com.c
)

target_include_directories(serverlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(serverlib PRIVATE
    network
)

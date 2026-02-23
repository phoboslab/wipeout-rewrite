
add_library(network
    addr_conversions.c
    msg.c
    network.c
    network_wrapper.c
)

set(OS_LIBS "")
if(WIN32)
    set(OS_LIBS "Ws2_32.lib;Iphlpapi.lib;${OS_LIBS}")
endif()

target_link_libraries(network PUBLIC ${OS_LIBS} protos)

target_include_directories(network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

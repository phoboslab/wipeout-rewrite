
file(GLOB PROTO_PROTOBUFS ${CMAKE_SOURCE_DIR}/protos/*.proto)

set(GENERATED_SRCS "")
set(GENERATED_HDRS "")

if(WIN32)
    set(PATHENV "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/protobuf-c;$ENV{PATH}")
else()
    set(PATHENV "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/protobuf-c:$ENV{PATH}")
endif()

foreach(PROTO_FILE ${PROTO_PROTOBUFS})

    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(GENERATED_SRC ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb-c.c)
    set(GENERATED_HDR ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb-c.h)

    list(APPEND GENERATED_SRCS ${GENERATED_SRC})
    list(APPEND GENERATED_HDRS ${GENERATED_HDR})

    add_custom_command(
        OUTPUT ${GENERATED_SRC} ${GENERATED_HDR}
        COMMAND ${CMAKE_COMMAND} -E env
            "PATH=${PATHENV}"
            ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/protobuf/protoc
        ARGS --c_out=${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_SOURCE_DIR}/protos ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Running protoc to generate ${GENERATED_SRC} and ${GENERATED_HDR}"
        VERBATIM
    )
endforeach(PROTO_FILE ${PROTO_PROTOBUFS})


# Custom target (optional: for IDEs or manual builds)
add_custom_target(generate_proto_files
    DEPENDS ${GENERATED_SRCS} ${GENERATED_HDRS}
)

add_library(protos STATIC
    ${GENERATED_SRCS}
)

add_dependencies(protos generate_proto_files)

target_link_libraries(protos PUBLIC PkgConfig::libprotobuf-c)

target_include_directories(protos PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
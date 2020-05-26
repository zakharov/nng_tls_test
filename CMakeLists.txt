cmake_minimum_required(VERSION 3.16)
project(nng_tls_test)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)

# Looks for a sanitizer
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(CheckSanitizer)
CheckSanitizer()
if (NOT MCX_SANITIZER STREQUAL "none")
    set(MCX_SANITIZER_FLAGS "-fsanitize=${MCX_SANITIZER}")
    message(STATUS "Using sanitizer: ${MCX_SANITIZER}")
endif ()

add_executable(nng_tls_server server.cpp)
target_link_libraries(nng_tls_server nng ${MCX_SANITIZER_FLAGS})
target_compile_options(nng_tls_server PUBLIC ${MCX_SANITIZER_FLAGS})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/server.pem ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(nng_tls_client client.cpp)
target_link_libraries(nng_tls_client nng Threads::Threads)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/client.crt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)


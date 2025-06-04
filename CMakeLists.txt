cmake_minimum_required(VERSION 3.20)
project(p2p_chat)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(p2p_chat src/p2p_chat.cpp)
add_executable(relay_server src/relay_server.cpp)

if(WIN32)
    target_link_libraries(p2p_chat ws2_32)
    target_link_libraries(relay_server ws2_32)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(p2p_chat PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(relay_server PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(p2p_chat PRIVATE /W4)
    target_compile_options(relay_server PRIVATE /W4)
endif()

set_target_properties(p2p_chat relay_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
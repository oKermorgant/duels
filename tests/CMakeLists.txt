cmake_minimum_required(VERSION 2.8.3)
project(duels CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
find_package (Threads)

file(GLOB HEADERS
    *.h)
include_directories(. games)

add_executable(comm_test communicator.cpp ${HEADERS})
set_target_properties(comm_test PROPERTIES LINKER_LANGUAGE CXX)

#add_executable(msg_test test/msg_test.cpp game_msg.h)

add_executable(client test/client.cpp player_client.h)

add_executable(server test/server.cpp game_server.h)
target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT})

add_executable(server_ex server_ex.cpp)
add_executable(client_ex client_ex.cpp)

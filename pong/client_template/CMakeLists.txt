cmake_minimum_required(VERSION 2.8.3)
project(pong CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

# find ZeroMQ
find_path ( ZMQ_INCLUDE_DIR zmq.h )
find_library ( ZMQ_LIBRARY NAMES zmq )
include_directories(/home/olivier/code/projects/duels/opt/duels//include ../include ${ZMQ_INCLUDE_DIR})

# add whatever additional includes / libs needed for your AI

add_executable(pong pong.cpp)
target_link_libraries(pong ${ZMQ_LIBRARY})

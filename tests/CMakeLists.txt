cmake_minimum_required(VERSION 2.8.3)
project(duels CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
add_compile_options(-pthread)
find_package (Threads)

# find ZeroMQ
find_path ( ZMQ_INCLUDE_DIR zmq.h )
find_library ( ZMQ_LIBRARY NAMES zmq )

include_directories(../include ${ZMQ_INCLUDE_DIR})

add_executable(a_star a_star.cpp)
target_link_libraries(a_star ${CMAKE_THREAD_LIBS_INIT} ${ZMQ_LIBRARY})

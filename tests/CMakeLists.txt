cmake_minimum_required(VERSION 3.1)
project(duels_tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-pthread)
find_package (Threads)
find_package(yaml-cpp)

# find ZeroMQ
find_path ( ZMQ_INCLUDE_DIR zmq.h )
find_library ( ZMQ_LIBRARY NAMES zmq )

include_directories(../include ${ZMQ_INCLUDE_DIR})

add_executable(a_star a_star.cpp)
target_link_libraries(a_star ${CMAKE_THREAD_LIBS_INIT} ${ZMQ_LIBRARY})

add_executable(yaml_parser yaml_parser.cpp)
target_link_libraries(yaml_parser ${CMAKE_THREAD_LIBS_INIT} ${ZMQ_LIBRARY} yaml-cpp)

cmake_minimum_required(VERSION 3.1)
project(fibo CXX)

# will be changed when creating package - used for local testing without installation
SET(DUELS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." CACHE STRING "Path to duels installation folder")
add_definitions(-DGAME_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/..")
include_directories(../include) # local dev

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# find ZeroMQ
find_path ( ZMQ_INCLUDE_DIR zmq.h )
find_library ( ZMQ_LIBRARY NAMES zmq )
# find yaml-cpp
find_package(PkgConfig)
pkg_check_modules(YamlCpp yaml-cpp)

include_directories(${DUELS_ROOT}/include ${ZMQ_INCLUDE_DIR} ${YamlCpp_INCLUDE_DIRS})
add_definitions(-DDUELS_BIN_PATH="${DUELS_ROOT}/bin")

# add whatever additional includes / libs needed for your AI

add_executable(fibo fibo.cpp)
target_link_libraries(fibo ${ZMQ_LIBRARY} yaml-cpp)
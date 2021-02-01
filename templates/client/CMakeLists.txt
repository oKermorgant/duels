cmake_minimum_required(VERSION 2.8.3)
project(<game> CXX)

# will be changed when creating package - used for local testing without installation
SET(DUELS_ROOT "<duels_path>" CACHE STRING "Path to duels installation folder")
add_definitions(-DGAME_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/..")
include_directories(../include) # local dev

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# find ZeroMQ
find_path ( ZMQ_INCLUDE_DIR zmq.h )
find_library ( ZMQ_LIBRARY NAMES zmq )
include_directories(${DUELS_ROOT}/include ${ZMQ_INCLUDE_DIR})
add_definitions(-DDUELS_BIN_PATH="${DUELS_ROOT}/bin")

# add whatever additional includes / libs needed for your AI

add_executable(<game> <game>.cpp)
target_link_libraries(<game> ${ZMQ_LIBRARY})

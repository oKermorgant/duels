cmake_minimum_required(VERSION 2.8.3)
project(pods CXX)

set(CMAKE_INSTALL_PREFIX ../../opt/duels)
set(DUELS_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

include_directories(../include ${DUELS_INCLUDE_DIR})

# client test will not be installed
add_executable(pods pods.cpp)

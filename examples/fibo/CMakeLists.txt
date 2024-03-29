cmake_minimum_required(VERSION 3.1)
project(fibo_server CXX)

SET(DUELS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE STRING "Path to duels dev folder")
set(CMAKE_INSTALL_PREFIX ${DUELS_ROOT})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
add_compile_options(-pthread)
find_package (Threads)

# find ZeroMQ
find_path ( ZMQ_INCLUDE_DIR zmq.h )
find_library ( ZMQ_LIBRARY NAMES zmq )
# find yaml-cpp
find_package(PkgConfig)
pkg_check_modules(YamlCpp yaml-cpp)

add_definitions(-DDUELS_BIN_PATH="${DUELS_ROOT}/bin")
add_definitions(-DGAME_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DDUELS_SERVER)

add_executable(fibo_server server.cpp)
target_include_directories(fibo_server PRIVATE 
    include
    ${DUELS_ROOT}/include 
    ${ZMQ_INCLUDE_DIR}
    ${YamlCpp_INCLUDE_DIRS})
target_link_libraries(fibo_server ${CMAKE_THREAD_LIBS_INIT} ${ZMQ_LIBRARY} yaml-cpp)

# add whatever is needed to build display executable


# install server
install(TARGETS fibo_server DESTINATION bin/fibo)

# install gui and its deps
install(FILES fibo_gui.py DESTINATION bin/fibo
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        
# client headers and templates
install(FILES include/duels/fibo/game.h include/duels/fibo/msg.h DESTINATION include/duels/fibo)
install(FILES client_template/CMakeLists.txt DESTINATION games/fibo)
install(FILES client_template/fibo.cpp DESTINATION games/fibo )
cmake_minimum_required(VERSION 3.1)
project(<game>_server CXX)

set(DUELS_ROOT "<duels_path>" CACHE STRING "Path to duels dev folder")
set(CMAKE_INSTALL_PREFIX ${DUELS_ROOT})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-pthread)
find_package (Threads)

# find ZeroMQ and yaml-cpp for message passing
find_path ( ZMQ_INCLUDE_DIR zmq.h )
find_library ( ZMQ_LIBRARY NAMES zmq )
find_package(yaml-cpp)

add_definitions(-DDUELS_BIN_PATH="${DUELS_ROOT}/bin")
add_definitions(-DGAME_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DDUELS_SERVER)

add_executable(<game>_server server.cpp)
target_include_directories(<game>_server PRIVATE 
    include
    ${DUELS_ROOT}/include 
    ${ZMQ_INCLUDE_DIR}
    ${YamlCpp_INCLUDE_DIRS})
target_link_libraries(<game>_server ${CMAKE_THREAD_LIBS_INIT} ${ZMQ_LIBRARY} yaml-cpp)

# add whatever is needed to build display executable


# install server
install(TARGETS <game>_server DESTINATION bin/<game>)

# install gui and its deps
install(FILES <game>_gui.py DESTINATION bin/<game>
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/<game>")
    # Python module
    install(DIRECTORY <game> DESTINATION bin/<game>)
endif()
        
# client headers and templates
install(FILES include/duels/<game>/game.h include/duels/<game>/msg.h <msg_detail> DESTINATION include/duels/<game>)
install(FILES client_template/CMakeLists.txt DESTINATION games/<game>)
install(FILES client_template/<game>.cpp DESTINATION games/<game> )

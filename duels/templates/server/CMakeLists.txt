cmake_minimum_required(VERSION 2.8.3)
project(<game>_server CXX)

set(CMAKE_INSTALL_PREFIX <duels_path>)
set(DUELS_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-pthread)
find_package (Threads)

# find ZeroMQ
find_path ( ZMQ_INCLUDE_DIR zmq.h )
find_library ( ZMQ_LIBRARY NAMES zmq )

include_directories(include ${DUELS_INCLUDE_DIR} ${ZMQ_INCLUDE_DIR})

add_executable(<game>_server server.cpp)
target_link_libraries(<game>_server ${CMAKE_THREAD_LIBS_INIT} ${ZMQ_LIBRARY})

# add whatever is needed to build display executable


# install server / display and copy client template
install(TARGETS <game>_server DESTINATION bin)
install(FILES <game>_gui DESTINATION bin
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES client_template/CMakeLists.txt DESTINATION games/<game>)
install(FILES client_template/<game>.cpp DESTINATION games/<game> )

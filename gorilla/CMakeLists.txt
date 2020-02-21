cmake_minimum_required(VERSION 2.8.3)
project(gorilla_server CXX)

set(CMAKE_INSTALL_PREFIX /home/olivier/code/projects/duels/opt/duels/)
set(DUELS_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-pthread)
find_package (Threads)

# find ZeroMQ
find_path ( ZMQ_INCLUDE_DIR zmq.h )
find_library ( ZMQ_LIBRARY NAMES zmq )

include_directories(include ../duels/include ${ZMQ_INCLUDE_DIR})

add_executable(gorilla_server server.cpp src/gorilla.cpp)
target_link_libraries(gorilla_server ${CMAKE_THREAD_LIBS_INIT} ${ZMQ_LIBRARY})

# add whatever is needed to build display executable


# install server / display and copy client template
install(TARGETS gorilla_server DESTINATION bin)
install(FILES gorilla_gui DESTINATION bin
        PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(DIRECTORY include/duels/gorilla DESTINATION include/duels)
install(FILES client_template/CMakeLists.txt DESTINATION games/gorilla)
install(FILES client_template/gorilla.cpp DESTINATION games/gorilla)

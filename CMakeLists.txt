cmake_minimum_required(VERSION 2.8)

project(Lane)

# find_package(yaml-cpp REQUIRED)
find_library(yaml yaml-cpp)
find_library(pthread-lib pthread)

include_directories(${YAML_CPP_INCLUDE_DIR})

include_directories(./src)

set(LIB_LIST ${pthread-lib} dl ${yaml})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

file(GLOB_RECURSE SRC_LIST
    src/*.cc
    src/**/*.cc)

# set(SRC_LIST
#     src/base/log.cc
#     src/base/AsyncLog.cc
#     src/base/util.cc
#     src/base/config.cc
#     src/base/thread.cc
#     src/base/mutex.cc
#     src/base/fiber.cc
#     src/base/scheduler.cc
#     src/base/iomanager.cc
#     src/base/timer.cc
#     src/base/fdmanager.cc
#     src/base/hook.cc
#     src/net/address.cc
#     src/net/socket.cc
#     src/net/socket.cc
#     src/net/stream.cc
#     src/net/socketstream.cc
#     src/net/bytearray.cc
#     src/http/http.cc
#     src/http/httpparser.cc
#     src/http/httpsession.cc
#     src/http/http11_parser.rl.cc
#     src/http/httpclient_parser.rl.cc
#     src/net/tcpserver.cc
#     src/http/httpserver.cc
#     src/http/servlet.cc
#     src/init/daemon.cc
#     src/init/env.cc
#     src/init/application.cc
#     src/base/worker.cc
#     src/init/module.cc
#     src/init/library.cc
#     src/base/worker.cc)
message(STATUS "-----SRC_LIST:${SRC_LIST}-----LIB_LIST:${LIB_LIST}---YAML-CPP:${yaml}")
add_compile_options(-std=c++11 -Wall -g -o0)

set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
add_library (laneLib STATIC ${SRC_LIST})
# set_target_properties (laneLib PROPERTIES OUTPUT_NAME "laneLib")
target_link_libraries(laneLib yaml-cpp ${LIB_LIST})

add_executable(test_log tests/test_log.cc)
target_link_libraries(test_log laneLib)

add_executable(test_config tests/test_config.cc)
target_link_libraries(test_config laneLib)

add_executable(test_thread tests/test_thread.cc)
target_link_libraries(test_thread  laneLib)

add_executable(test_fiber tests/test_fiber.cc)
target_link_libraries(test_fiber  laneLib)

add_executable(test_scheduler tests/test_scheduler.cc)
target_link_libraries(test_scheduler  laneLib)

add_executable(test_iomanager tests/test_iomanager.cc)
target_link_libraries(test_iomanager  laneLib)

add_executable(test_hook tests/test_hook.cc)
target_link_libraries(test_hook  laneLib)

add_executable(test_address tests/test_address.cc)
target_link_libraries(test_address  laneLib)

add_executable(test_socket tests/test_socket.cc)
target_link_libraries(test_socket  laneLib)

add_executable(test_tcpserver tests/test_tcpserver.cc)
target_link_libraries(test_tcpserver  laneLib)

add_executable(test_httpparser tests/test_httpparser.cc)
target_link_libraries(test_httpparser  laneLib)

add_executable(test_httpserver tests/test_httpserver.cc)
target_link_libraries(test_httpserver  laneLib)

add_executable(test_daemon tests/test_daemon.cc)
target_link_libraries(test_daemon  laneLib)

add_executable(test_application tests/test_application.cc)
target_link_libraries(test_application  laneLib)

add_executable(test_mutex tests/test_mutex.cc)
target_link_libraries(test_mutex  laneLib)

add_executable(test_AsyncLog tests/test_AsyncLog.cc)
target_link_libraries(test_AsyncLog  laneLib)


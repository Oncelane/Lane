cmake_minimum_required(VERSION 2.8)

project(Lane)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
# find_package(yaml-cpp REQUIRED)
find_library(yaml yaml-cpp)
find_library(pthread-lib pthread)
find_package(Boost REQUIRED COMPONENTS context)

include_directories(${YAML_CPP_INCLUDE_DIR})

include_directories(./src)

set(LIB_LIST ${pthread-lib} dl ${yaml})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

file(GLOB_RECURSE SRC_LIST
    src/*.cc
    src/**/*.cc)


message(STATUS "-----SRC_LIST:${SRC_LIST}-----LIB_LIST:${LIB_LIST}---YAML-CPP:${yaml}")
add_compile_options(-std=c++11 -Wall -g -O3)

set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
add_library (laneLib STATIC ${SRC_LIST})
# set_target_properties (laneLib PROPERTIES OUTPUT_NAME "laneLib")
target_link_libraries(laneLib Boost::context ${LIB_LIST} yaml-cpp   mysqlclient)

# 自动查找测试文件
file(GLOB TEST_SOURCES tests/*.cc)
foreach(TEST_SOURCE ${TEST_SOURCES})
    # 获取文件名，无扩展名
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    # 创建可执行文件
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    # 链接库
    target_link_libraries(${TEST_NAME} laneLib)
endforeach()
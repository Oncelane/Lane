cmake_minimum_required(VERSION 3.0)

project(Lane)
set(CMAKE_BUILD_TYPE Debug)

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

    function(force_redefine_file_macro_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        # Get source file's current list of compile definitions.
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS)
        # Get the relative path of the source file in project directory
        get_filename_component(filepath "${sourcefile}" ABSOLUTE)
        string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
        list(APPEND defs "__FILE__=\"${relpath}\"")
        # Set the updated compile definitions on the source file.
        set_property(
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs}
            )
    endforeach()
endfunction()

message(STATUS "-----SRC_LIST:${SRC_LIST}-----LIB_LIST:${LIB_LIST}---YAML-CPP:${yaml}")
add_compile_options(-std=c++17 -Wall -O3 -flto=auto)
# add_compile_options(-std=c++17 -Wall -fsanitize=address -g -O0)

set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
add_library (laneLib STATIC ${SRC_LIST})
force_redefine_file_macro_for_sources(laneLib)

# ===================为 laneLib 设置 AddressSanitizer===================
# set_target_properties(laneLib PROPERTIES LINK_FLAGS "-fsanitize=address")

# set_target_properties (laneLib PROPERTIES OUTPUT_NAME "laneLib")
target_link_libraries(laneLib Boost::context ${LIB_LIST} yaml-cpp   mysqlclient)



# 自动查找测试文件
file(GLOB TEST_SOURCES tests/*.cc)
foreach(TEST_SOURCE ${TEST_SOURCES})
    # 获取文件名，无扩展名
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    # 创建可执行文件
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    force_redefine_file_macro_for_sources(${TEST_NAME}) #__FILE__

 # ===================为测试可执行文件设置 AddressSanitizer===================
#  set_target_properties(${TEST_NAME} PROPERTIES LINK_FLAGS "-fsanitize=address")

    # 链接库
    target_link_libraries(${TEST_NAME} laneLib)
endforeach()

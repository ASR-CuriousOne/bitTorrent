cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_library(core_lib STATIC ${CORE_SOURCES})

target_include_directories(core_lib PRIVATE "${CMAKE_SOURCE_DIR}/include")

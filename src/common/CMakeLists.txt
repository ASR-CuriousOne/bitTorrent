cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_library(core STATIC ${CORE_SOURCES})

target_link_libraries(core PRIVATE logger)

target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}/include")

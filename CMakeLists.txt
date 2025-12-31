cmake_minimum_required(VERSION 3.10)

project(bitTorrent VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src/core)
add_subdirectory(vendor/logger)

add_executable(torrent src/main.cpp)

target_link_libraries(torrent PRIVATE core_lib)
target_link_libraries(torrent PRIVATE logger)

target_include_directories(torrent PRIVATE
				"${CMAKE_SOURCE_DIR}/include"
)

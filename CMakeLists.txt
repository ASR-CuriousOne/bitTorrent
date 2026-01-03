cmake_minimum_required(VERSION 3.20)

project(bitTorrent VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendor/logger)
add_subdirectory(src/common)
add_subdirectory(src/client)
add_subdirectory(src/server)

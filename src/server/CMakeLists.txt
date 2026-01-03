cmake_minimum_required(VERSION 3.20)

file(GLOB SERVER_SRCS "*.cpp")

add_executable(server ${SERVER_SRCS})

target_link_libraries(server PRIVATE core logger)

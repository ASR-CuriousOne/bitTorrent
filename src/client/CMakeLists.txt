cmake_minimum_required(VERSION 3.20)

file(GLOB CLIENT_SRCS "*.cpp")

add_executable(client ${CLIENT_SRCS})

target_link_libraries(client PRIVATE core logger)

cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
set(This chess)

project(${This} CXX C)

add_subdirectory(src)
add_subdirectory(include)

include_directories(
    include
    /usr/include
    g
)
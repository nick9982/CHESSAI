cmake_minimum_required(VERSION 3.16)

project(chess)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(chess_game)

target_link_libraries(${PROJECT_NAME} chess_game)
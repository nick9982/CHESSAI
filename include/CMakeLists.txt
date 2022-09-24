cmake_minimum_required(VERSION 3.16)

project(chess)

add_subdirectory(chess_game)

target_include_directories(${This} PRIVATE ./)
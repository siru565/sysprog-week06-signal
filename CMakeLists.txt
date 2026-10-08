cmake_minimum_required(VERSION 3.16)
project(week06_signal C)

set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Wextra)

add_executable(1_sigint3 1_sigint3.c)
add_executable(alarm alarm.c)
add_executable(3_signal_block2 3_signal_block2.c)

cmake_minimum_required(VERSION 3.9)
project(ex3 C)

set(CMAKE_C_STANDARD 99)

add_library(ex3 ex3_2.c ex3_2.h ex3_1.c ex3_1.h)
#add_executable(a.out ex3_2.c ex3_2.h)
add_executable(a.out ex3_2.c ex3_2.h)
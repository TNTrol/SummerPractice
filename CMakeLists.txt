cmake_minimum_required(VERSION 3.19)
project(SummerPracticle C)

set(CMAKE_C_STANDARD 99)

add_executable(SummerPracticle main.c utils.c utils.h)

target_link_libraries(${PROJECT_NAME} PUBLIC ssl crypto )
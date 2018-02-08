cmake_minimum_required(VERSION 3.9)
project(ThreadSafeQueue C)

set(CMAKE_C_STANDARD 99)
SET(CMAKE_EXE_LINKER_FLAGS "-pthread")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

add_executable(ThreadSafeQueue main.c)

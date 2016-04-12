project(jitgdb)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE true)

set(CMAKE_C_FLAGS "-nostartfiles")

add_library(jitgdb SHARED jitgdb.c)


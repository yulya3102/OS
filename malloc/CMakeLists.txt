cmake_minimum_required(VERSION 2.8)
project(malloc)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall ${CMAKE_CXX_FLAGS}")

set(SOURCES
    mmap.h   mmap.cpp
    linear.h linear.cpp
    common.h common.cpp
    malloc.cpp
)

add_library(malloc SHARED ${SOURCES})

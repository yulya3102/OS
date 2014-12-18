cmake_minimum_required(VERSION 2.8)
project(malloc)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall ${CMAKE_CXX_FLAGS}")

set(SOURCES
    mmap.h   mmap.cpp
    slab.h   slab.cpp
    hoard.h  hoard.cpp
    common.h common.cpp
    malloc.cpp
)

add_library(malloc SHARED ${SOURCES})

find_package(GTest)

set(TESTS
    test.cpp
)

add_executable(test_malloc ${TESTS})

target_link_libraries(test_malloc ${GTEST_LIBRARY})

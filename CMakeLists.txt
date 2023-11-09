cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0091 NEW)

project(gbemu)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

if (DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
endif()

add_subdirectory(vendor)
add_subdirectory(src)
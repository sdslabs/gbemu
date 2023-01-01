cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0091 NEW)

project(gbemu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")

if(MSVC)
    add_compile_options(/MP)
endif()

add_executable(${PROJECT_NAME} src/main.cpp)
add_subdirectory(src)

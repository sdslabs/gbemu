set(SOURCES
        # -------
        # Source Files
        cpu.cpp
        gameBoy.cpp
        mmap.cpp
        graphics.cpp
        # -------
        # Header Files
        cpu.h
        gameBoy.h
        mmap.h
        ../common/types.h
        graphics.h
        )

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

if (MSVC)
    set_target_properties(
            ${PROJECT_NAME} PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/")
endif ()

target_link_libraries(${PROJECT_NAME} gui_gbemu)

set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

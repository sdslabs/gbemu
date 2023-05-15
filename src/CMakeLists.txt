set(SOURCES
        # -------
        # Source Files
        cpu.cpp
        gameBoy.cpp
        mmap.cpp
        graphics.cpp
        apu.cpp

        # -------
        # Header Files
        cpu.h
        gameBoy.h
        mmap.h
        types.h
        graphics.h
        apu.h
        )

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

if (MSVC)
    set_target_properties(
            ${PROJECT_NAME} PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/")
endif ()

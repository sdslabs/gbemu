set(SOURCES
  gameBoy.h
  types.h
  )

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

if (MSVC)
    set_target_properties(
            ${PROJECT_NAME} PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/")
endif ()

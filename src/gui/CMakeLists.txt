set(HEADERS_GUI_GBEMU
        window.h
        dock/DockUI.h
        ui-flags/UIFlags.h
        gui.h
        menubar/MenuBar.h)
set(SOURCES_GUI_GBEMU
        window.cpp
        dock/DockUI.cpp
        ui-flags/UIFlags.cpp
        menubar/MenuBar.cpp
        )

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_library(gui_gbemu STATIC ${HEADERS_GUI_GBEMU} ${SOURCES_GUI_GBEMU})


target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})


if (MSVC)
    set_target_properties(
            gui_gbemu PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/")
endif ()

target_link_libraries(gui_gbemu
        PUBLIC ${SDL2_LIBRARIES}
        PUBLIC imgui)



set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")
target_include_directories(gui_gbemu PUBLIC
        ${PROJECT_SOURCE_DIR}/vendor
        ${PROJECT_SOURCE_DIR}/src
        )
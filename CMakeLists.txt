cmake_minimum_required(VERSION 3.10)
project(SolarSystem)

set(CMAKE_CXX_STANDARD 17)

#Include directories
include_directories(
    include
    external/imgui
    external/imgui/backends)

#local library directories
link_directories(lib)

#source files
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    src/*.c
)

#source file for imgui
set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

#add resources in the build directory
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})

#create executable
add_executable(SolarSystem 
        ${SRC_FILES}
        ${IMGUI_SRC})

target_link_libraries(SolarSystem
    glfw3
    opengl32
    gdi32
)
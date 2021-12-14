# Only for cmake --version >= 3.5.1
cmake_minimum_required(VERSION 3.5.1)

# Project name
project(chip-8)

# Creates the variable EXEC and sets it to chip8
set(EXEC chip8)

# set the C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Set the submodule directory
SET(SUBMODULE_DIR submodules)

# GLFW (https://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_source)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${SUBMODULE_DIR}/glfw)

# I../includes
include_directories(includes)
include_directories(${SUBMODULE_DIR}/olcPixelGameEngine)
include_directories(${SUBMODULE_DIR}/glfw/include)
include_directories(${SUBMODULE_DIR}/imgui)
include_directories(${SUBMODULE_DIR}/imgui/backends)

# Remove OpenGL deprecation warning
add_compile_definitions(GL_SILENCE_DEPRECATION)

find_package(OpenGL REQUIRED)

# Puts all .cpp files inside src
file(GLOB SOURCES_CHIP8 src/*.cpp)

# Put imgui .cpp files to sources
file(GLOB SOURCES_IMGUI ${SUBMODULE_DIR}/imgui/*.cpp)

set(SOURCES
  ${SOURCES_CHIP8}
  ${SOURCES_IMGUI}
  # Add the GLFW backend
  ${SUBMODULE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
  ${SUBMODULE_DIR}/imgui/backends/imgui_impl_glfw.cpp
)

# Compiles the files defined by SOURCES to generante the executable defined by EXEC
add_executable(${EXEC} ${SOURCES})

# Link with glfw
target_link_libraries(${EXEC} glfw)
target_link_libraries(${EXEC} OpenGL::GL)

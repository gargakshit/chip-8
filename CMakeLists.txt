# Only for cmake --version >= 3.5.1
cmake_minimum_required(VERSION 3.5.1)

# Project name
project(chip-8)

# Creates the variable EXEC and sets it to hello
set(EXEC chip8)

# set the C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Set the submodule directory
SET(SUBMODULE_DIR submodules)

# I../includes
include_directories(includes)
include_directories(${SUBMODULE_DIR}/olcPixelGameEngine)

# Puts all .cpp files inside src to the SOURCES variable
file(GLOB SOURCES src/*.cpp)

# Compiles the files defined by SOURCES to generante the executable defined by EXEC
add_executable(${EXEC} ${SOURCES})

cmake_minimum_required(VERSION 3.16)
project(liverun)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: show warnings
add_compile_options(-Wall -Wextra -Werror)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Gather all source files recursively
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Define executable
add_executable(liverun ${SOURCES})


cmake_minimum_required(VERSION 3.16)
project(liverun)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Gather all source files recursively
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(liverun ${SOURCES})

file(GLOB_RECURSE LIVERN_SRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_library(livern STATIC ${LIVERN_SRC})

# Make headers in src/ visible to others
target_include_directories(livern PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(tests)

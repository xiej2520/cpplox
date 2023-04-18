cmake_minimum_required(VERSION 3.7)
project(CppLox VERSION 1.0 DESCRIPTION "CppLox" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include_directories(include)
link_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp" main.cpp)

add_executable(lox ${SOURCES})
target_compile_options(lox PRIVATE -Wall -Wextra -Werror -Wpedantic)


# Debug build
#cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Debug
#cmake --build build/

# Release build
#cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Release
#cmake --build build/
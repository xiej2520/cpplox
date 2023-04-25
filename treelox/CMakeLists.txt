cmake_minimum_required(VERSION 3.7)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(CppLox VERSION 1.0 DESCRIPTION "CppLox" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include_directories(include)
link_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp" main.cpp)

if(MSVC)
    message(STATUS "MSVC detected")
    add_compile_options("/W4" "$<$<CONFIG:RELEASE>:/O2>")
else()
    message(STATUS "Non-MSVC compiler detected")
    # -gdwarf-4 for valgrind?
    add_compile_options("-Wall" "-Wextra" "-Werror" "$<$<CONFIG:Debug>:-gdwarf-4>" "$<$<CONFIG:Release>:-O3>")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options("$<$<CONFIG:Debug>:-g3>" "-fdiagnostics-format=clang" "-ftemplate-backtrace-limit=1024" "-ftemplate-backtrace-limit=32")
        #add_compile_options("-stdlib=libc++")
    else()
    endif()
endif()

# see commands in build/compile_commands.json

# comes after add_compile_options
add_executable(lox ${SOURCES})

set(CMAKE_CXX_CLANG_TIDY
    "clang-tidy;-header-filter=.*")

# Debug build
#cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Debug
#cmake --build build/

# Release build
#cmake -S . -B build/ -D CMAKE_BUILD_TYPE=Release
#cmake --build build/

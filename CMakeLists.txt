cmake_minimum_required(VERSION 3.1...3.29)

project(biner VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    include
)

add_executable(biner
    src/biner.cpp
)

install(TARGETS biner DESTINATION bin)

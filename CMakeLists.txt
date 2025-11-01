cmake_minimum_required(VERSION 3.15)
project(my_b LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(${PROJECT_NAME} ${SOURCES})

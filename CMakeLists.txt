cmake_minimum_required(VERSION 3.15)
project(my_b LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Google test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/*.cpp")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(REMOVE_ITEM SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")

add_library(my_b_lib ${SOURCES})

# Apply compile options to library
target_compile_options(my_b_lib PRIVATE -Wall -Wextra -Wpedantic -Werror)

# Debug definition
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug ON")
    target_compile_definitions(my_b_lib PUBLIC DEBUG=1)
else ()
    message(STATUS "Debug off")
endif()

find_package(OpenSSL REQUIRED)
target_link_libraries(my_b_lib OpenSSL::SSL OpenSSL::Crypto)

# Main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} my_b_lib)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

# Test executable
enable_testing()
add_executable(tests ${TEST_SOURCES})
target_compile_definitions(tests PRIVATE TEST_DATA_DIR="${PROJECT_SOURCE_DIR}/tests")
target_link_libraries(tests my_b_lib GTest::gtest_main gmock)
target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic -Werror)

include(GoogleTest)
gtest_discover_tests(tests)

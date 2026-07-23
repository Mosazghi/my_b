add_subdirectory(ui)
add_subdirectory(lib)

# Move executables to the root of the build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(my_b main.cpp)

target_link_libraries(my_b
        PRIVATE
        MyBLib
)

target_compile_options(my_b PRIVATE -Wall -Wextra -Wpedantic)

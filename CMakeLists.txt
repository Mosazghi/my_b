cmake_minimum_required(VERSION 3.16)
project(my_b LANGUAGES CXX)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ZLIB CONFIG REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc)
find_package(OpenSSL CONFIG REQUIRED)
find_package(SFML 2.6 CONFIG REQUIRED COMPONENTS graphics window system)
find_package(GTest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

FetchContent_Declare(
     imgui
     GIT_REPOSITORY https://github.com/ocornut/imgui.git
     GIT_TAG v1.90.9
 )
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
     imgui-sfml
     GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
     GIT_TAG 2.6.x
     PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/patch_imgui_sfml.cmake ${CMAKE_BINARY_DIR}/_deps/imgui-sfml-src/imgui-SFML.cpp
     UPDATE_DISCONNECTED 1
 )
set(IMGUI_DIR ${imgui_SOURCE_DIR} CACHE PATH "" FORCE)
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(imgui-sfml)
add_library(imgui STATIC
     ${imgui_SOURCE_DIR}/imgui.cpp
     ${imgui_SOURCE_DIR}/imgui_draw.cpp
     ${imgui_SOURCE_DIR}/imgui_widgets.cpp
     ${imgui_SOURCE_DIR}/imgui_tables.cpp
     ${imgui_SOURCE_DIR}/imgui_demo.cpp
 )
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "ccache found: ${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

set(SOURCES
    src/browser/Browser.cpp
    src/file/File.cpp
    src/http/HttpClient.cpp
    src/http/HttpRequest.cpp
    src/layout/layout.cpp
    src/resource-manager/ResourceManager.cpp
    src/ui/Scrollbar.cpp
    src/url/Url.cpp
    src/common.cpp
    src/utils.cpp
)

add_library(my_b_lib OBJECT ${SOURCES})
target_include_directories(my_b_lib PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_compile_options(my_b_lib PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug ON")
    target_compile_definitions(my_b_lib PRIVATE DEBUG=1)
else()
    message(STATUS "Debug off")
endif()

target_link_libraries(my_b_lib
    PRIVATE
        OpenSSL::SSL OpenSSL::Crypto
        ZLIB::ZLIB
        sfml-graphics sfml-window sfml-system
        ICU::uc
        ImGui-SFML::ImGui-SFML
)
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        my_b_lib
        ImGui-SFML::ImGui-SFML
        fmt::fmt
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

# Tests live in their own directory
include(CTest)
enable_testing()
add_subdirectory(tests)

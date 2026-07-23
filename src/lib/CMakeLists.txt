project(MyBLib)

include(FetchContent)

find_package(SFML 2.6 CONFIG REQUIRED COMPONENTS graphics window system)
find_package(ZLIB REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc)
find_package(OpenSSL REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)


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

set(LIB_SOURCES
    browser/Browser.cpp
    file/File.cpp
    http/HttpClient.cpp
    http/HttpRequest.cpp
    layout/layout.cpp
    resource-manager/ResourceManager.cpp
    resource-loader/ResourceLoader.cpp
    url/Url.cpp
    common/common.cpp
    utils/utils.cpp
)

add_library(MyBLib ${LIB_SOURCES})

target_include_directories(MyBLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


target_compile_options(MyBLib PRIVATE -Wall -Wextra -Wpedantic)
if(ENABLE_COVERAGE)
  target_compile_options(MyBLib PUBLIC --coverage -O0 -g)
  target_link_options(MyBLib PUBLIC --coverage)
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Debug ON")
  target_compile_definitions(MyBLib PRIVATE DEBUG)
else()
  message(STATUS "Debug off")
endif()


target_link_libraries(MyBLib
        PUBLIC
        MyBUI
        OpenSSL::SSL OpenSSL::Crypto
        ZLIB::ZLIB
        ICU::uc
        ImGui-SFML::ImGui-SFML
        magic_enum::magic_enum
        fmt::fmt
)

target_compile_options(MyBLib PRIVATE -Wall -Wextra -Wpedantic)

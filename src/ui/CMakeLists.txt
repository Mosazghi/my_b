project(MyBUI)

set(UI_SOURCES
    Button.cpp
    Button.hpp
    RoundedRectangle.cpp
    RoundedRectangle.hpp
    Scrollbar.cpp
    Scrollbar.hpp
    UiManager.cpp
    UiManager.hpp
)

add_library(MyBUI ${UI_SOURCES})

target_include_directories(MyBUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(MyBUI
    PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)
target_compile_options(MyBUI PRIVATE -Wall -Wextra -Wpedantic -Werror)

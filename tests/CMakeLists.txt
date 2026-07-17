
set(TEST_SOURCES
    test_file.cpp
    test_http_client.cpp
    test_url.cpp
    test_utilites.cpp
)
add_executable(tests ${TEST_SOURCES})

target_compile_definitions(tests PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(tests
    PRIVATE
        my_b_lib
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
)

target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic -Werror)

include(GoogleTest)
gtest_discover_tests(tests)

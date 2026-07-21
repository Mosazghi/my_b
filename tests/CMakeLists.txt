set(TEST_SOURCES
    test_common.cpp
    test_file.cpp
    test_http_client.cpp
    test_http_request.cpp
    test_url.cpp
    test_utilites.cpp
)
add_executable(tests ${TEST_SOURCES})
if(ENABLE_COVERAGE)
  target_compile_options(tests PRIVATE --coverage -O0 -g)
  target_link_options(tests PRIVATE --coverage)
endif()



target_compile_definitions(tests PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(tests
    PRIVATE
        MyBLib
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
)

target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic -Werror)

include(GoogleTest)
gtest_discover_tests(tests)

cmake_minimum_required(VERSION 3.20)
project(polycpp_moment VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Fetch polycpp (disable its tests when building as a dependency)
set(POLYCPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    polycpp
    GIT_REPOSITORY git@github.com:enricohuang/polycpp.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(polycpp)

# Library target
add_library(polycpp_moment
    src/moment.cpp
)
target_link_libraries(polycpp_moment PUBLIC polycpp)
target_include_directories(polycpp_moment PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Alias for consistent naming
add_library(polycpp::moment ALIAS polycpp_moment)

# Tests
option(POLYCPP_MOMENT_BUILD_TESTS "Build tests" ON)
if(POLYCPP_MOMENT_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY git@github.com:google/googletest.git
        GIT_TAG        v1.15.2
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(test_moment_basic tests/test_moment_basic.cpp)
    target_link_libraries(test_moment_basic PRIVATE polycpp_moment GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(test_moment_basic)

    add_executable(test_units tests/test_units.cpp)
    target_link_libraries(test_units PRIVATE polycpp_moment GTest::gtest_main)
    gtest_discover_tests(test_units)

    add_executable(test_locale tests/test_locale.cpp)
    target_link_libraries(test_locale PRIVATE polycpp_moment GTest::gtest_main)
    gtest_discover_tests(test_locale)

    add_executable(test_moment tests/test_moment.cpp)
    target_link_libraries(test_moment PRIVATE polycpp_moment GTest::gtest_main)
    gtest_discover_tests(test_moment)

    add_executable(test_duration tests/test_duration.cpp)
    target_link_libraries(test_duration PRIVATE polycpp_moment GTest::gtest_main)
    gtest_discover_tests(test_duration)

    add_executable(test_format tests/test_format.cpp)
    target_link_libraries(test_format PRIVATE polycpp_moment GTest::gtest_main)
    gtest_discover_tests(test_format)

    add_executable(test_parse tests/test_parse.cpp)
    target_link_libraries(test_parse PRIVATE polycpp_moment GTest::gtest_main)
    gtest_discover_tests(test_parse)

    add_executable(test_display tests/test_display.cpp)
    target_link_libraries(test_display PRIVATE polycpp_moment GTest::gtest_main)
    gtest_discover_tests(test_display)

    add_executable(test_query tests/test_query.cpp)
    target_link_libraries(test_query PRIVATE polycpp_moment GTest::gtest_main)
    gtest_discover_tests(test_query)

    add_executable(test_json_integration tests/test_json_integration.cpp)
    target_link_libraries(test_json_integration PRIVATE polycpp_moment GTest::gtest_main)
    gtest_discover_tests(test_json_integration)
endif()

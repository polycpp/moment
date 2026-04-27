cmake_minimum_required(VERSION 3.20)
project(polycpp_moment VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(cmake/PolycppMomentLocales.cmake)

# Fetch polycpp (disable its tests when building as a dependency)
set(POLYCPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(POLYCPP_SOURCE_DIR "" CACHE PATH "Path to a local polycpp checkout")

if(POLYCPP_SOURCE_DIR AND EXISTS "${POLYCPP_SOURCE_DIR}/CMakeLists.txt")
    FetchContent_Declare(polycpp SOURCE_DIR "${POLYCPP_SOURCE_DIR}")
else()
    FetchContent_Declare(
        polycpp
        GIT_REPOSITORY https://github.com/enricohuang/polycpp.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
endif()
FetchContent_MakeAvailable(polycpp)

# Library target
add_library(polycpp_moment
    src/moment.cpp
)
polycpp_moment_configure_locales(polycpp_moment)
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
        GIT_REPOSITORY https://github.com/google/googletest.git
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

    if(POLYCPP_MOMENT_ALL_GENERATED_LOCALES_ENABLED)
        add_executable(test_generated_locales tests/test_generated_locales.cpp)
        target_link_libraries(test_generated_locales PRIVATE polycpp_moment GTest::gtest_main)
        gtest_discover_tests(test_generated_locales)
    endif()

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

option(POLYCPP_MOMENT_BUILD_EXAMPLES "Build examples" OFF)
if(POLYCPP_MOMENT_BUILD_EXAMPLES)
    file(GLOB POLYCPP_MOMENT_EXAMPLES CONFIGURE_DEPENDS examples/*.cpp)
    foreach(example_source IN LISTS POLYCPP_MOMENT_EXAMPLES)
        get_filename_component(example_name "${example_source}" NAME_WE)
        add_executable("${example_name}" "${example_source}")
        target_link_libraries("${example_name}" PRIVATE polycpp_moment)
        set_target_properties("${example_name}" PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
        )
    endforeach()
endif()

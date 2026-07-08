cmake_minimum_required(VERSION 3.10)
project(pmbus_task LANGUAGES C CXX)

option(BUILD_TESTS "Build unit tests" ON)

add_library(firmware STATIC
    firmware/smbus_crc.c
    firmware/pmbus_read_ein.c
)
target_include_directories(firmware PUBLIC firmware)

if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(test_smbus_crc firmware/tests/test_smbus_crc.cpp)
    target_link_libraries(test_smbus_crc firmware gtest_main)
    add_test(NAME SMBusCRC COMMAND test_smbus_crc)

    add_executable(test_pmbus_read_ein firmware/tests/test_pmbus_read_ein.cpp)
    target_link_libraries(test_pmbus_read_ein firmware gtest_main)
    add_test(NAME PMBusReadEin COMMAND test_pmbus_read_ein)
endif()
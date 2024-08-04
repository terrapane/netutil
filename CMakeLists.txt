cmake_minimum_required(VERSION 3.21)

# Define the Network Utilities project
project(netutil
        VERSION 1.0.0.0
        DESCRIPTION "Network Utilities Library"
        LANGUAGES CXX)

# Tests are built by default when this is a top-level project
if(PROJECT_IS_TOP_LEVEL)
    # Option to control whether tests are built
    option(netutil_BUILD_TESTS "Build Tests for the Network Utilities Library" ON)
else()
    # Option to control whether tests are built
    option(netutil_BUILD_TESTS "Build Tests for the Network Utilities Library" OFF)
endif()

# Option to control ability to install the library
option(netutil_INSTALL "Install the Network Utilities Library" ON)

# Determine whether clang-tidy will be performed
option(netutil_CLANG_TIDY "Use clang-tidy to perform linting during build" OFF)

add_subdirectory(dependencies)
add_subdirectory(src)

include(CTest)

if(BUILD_TESTING AND netutil_BUILD_TESTS)
    add_subdirectory(test)
endif()

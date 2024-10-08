# Create the library
add_library(netutil STATIC
    data_buffer.cpp
    varint_data_buffer.cpp
    network_address.cpp)
add_library(Terra::netutil ALIAS netutil)

# Specify the internal and public include directories
target_include_directories(netutil
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

# Specify the C++ standard to observe
set_target_properties(netutil
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)

# If requesting clang-tidy, try to look for it
if(netutil_CLANG_TIDY)
    find_program(CLANG_TIDY_COMMAND NAMES "clang-tidy")
    if(CLANG_TIDY_COMMAND)
        set_target_properties(netutil PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
    else()
        message(WARNING "Could not find clang-tidy")
    endif()
endif()

# Use the following compile options
target_compile_options(netutil
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
        $<$<CXX_COMPILER_ID:MSVC>: >)

# Link against library dependencies
target_link_libraries(netutil PRIVATE Terra::bitutil)

if(WIN32)
    target_link_libraries(netutil PRIVATE Ws2_32)
endif()

# Install target and associated include files
if(netutil_INSTALL)
    include(GNUInstallDirs)
    install(TARGETS netutil EXPORT netutilTargets ARCHIVE)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ TYPE INCLUDE)
    install(EXPORT netutilTargets
            FILE netutilConfig.cmake
            NAMESPACE Terra::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/netutil)
endif()

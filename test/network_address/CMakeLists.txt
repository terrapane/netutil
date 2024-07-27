add_executable(test_network_address test_network_address.cpp)

target_link_libraries(test_network_address Terra::netutil Terra::stf)

# Specify the C++ standard to observe
set_target_properties(test_network_address
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)

target_compile_options(test_network_address
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
        $<$<CXX_COMPILER_ID:MSVC>: >)

add_test(NAME test_network_address
         COMMAND test_network_address)

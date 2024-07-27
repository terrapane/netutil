add_executable(test_data_buffer test_data_buffer.cpp)

target_link_libraries(test_data_buffer Terra::netutil Terra::stf)

# Specify the C++ standard to observe
set_target_properties(test_data_buffer
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)

target_compile_options(test_data_buffer
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
        $<$<CXX_COMPILER_ID:MSVC>: >)

add_test(NAME test_data_buffer
         COMMAND test_data_buffer)

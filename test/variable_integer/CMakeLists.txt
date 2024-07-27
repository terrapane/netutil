add_executable(test_variable_integer test_variable_integer.cpp)

target_link_libraries(test_variable_integer Terra::netutil Terra::stf)

# Specify the C++ standard to observe
set_target_properties(test_variable_integer
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)

target_compile_options(test_variable_integer
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wpedantic -Wextra -Wall>
        $<$<CXX_COMPILER_ID:MSVC>: >)

add_test(NAME test_variable_integer
         COMMAND test_variable_integer)

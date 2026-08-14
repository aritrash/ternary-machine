cmake_minimum_required(VERSION 3.25)

project(TernaryMachine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ternary_machine INTERFACE)
target_include_directories(ternary_machine INTERFACE ${PROJECT_SOURCE_DIR}/src)

enable_testing()

add_executable(trit_test tests/ternary/trit_test.cpp)
target_link_libraries(trit_test PRIVATE ternary_machine)
add_test(NAME trit_test COMMAND trit_test)

add_executable(tryte_test tests/ternary/tryte_test.cpp)
target_link_libraries(tryte_test PRIVATE ternary_machine)
add_test(NAME tryte_test COMMAND tryte_test)

add_executable(word_test tests/ternary/word_test.cpp)
target_link_libraries(word_test PRIVATE ternary_machine)
add_test(NAME word_test COMMAND word_test)

add_executable(instruction_test tests/ternary/instruction_test.cpp)
target_link_libraries(instruction_test PRIVATE ternary_machine)
add_test(NAME instruction_test COMMAND instruction_test)

add_executable(register_file_test tests/vm/register_file_test.cpp)
target_link_libraries(register_file_test PRIVATE ternary_machine)
add_test(NAME register_file_test COMMAND register_file_test)

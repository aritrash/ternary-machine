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

add_executable(cpu_state_test tests/vm/cpu_state_test.cpp)
target_link_libraries(cpu_state_test PRIVATE ternary_machine)
add_test(NAME cpu_state_test COMMAND cpu_state_test)

add_executable(memory_test tests/vm/memory_test.cpp)
target_link_libraries(memory_test PRIVATE ternary_machine)
add_test(NAME memory_test COMMAND memory_test)

add_executable(machine_test tests/vm/machine_test.cpp)
target_link_libraries(machine_test PRIVATE ternary_machine)
add_test(NAME machine_test COMMAND machine_test)

add_executable(executor_test tests/vm/executor_test.cpp)
target_link_libraries(executor_test PRIVATE ternary_machine)
add_test(NAME executor_test COMMAND executor_test)

add_executable(alu_test tests/vm/alu_test.cpp)
target_link_libraries(alu_test PRIVATE ternary_machine)
add_test(NAME alu_test COMMAND alu_test)

add_executable(context_test tests/vm/context_test.cpp)
target_link_libraries(context_test PRIVATE ternary_machine)
add_test(NAME context_test COMMAND context_test)

add_executable(vector_table_test tests/vm/vector_table_test.cpp)
target_include_directories(vector_table_test PRIVATE src)
add_test(NAME vector_table_test COMMAND vector_table_test)

add_executable(lexer_test tests/assembler/lexer_test.cpp)
target_include_directories(lexer_test PRIVATE src)
target_compile_features(lexer_test PRIVATE cxx_std_20)
add_test(NAME lexer_test COMMAND lexer_test)

add_executable(parser_test tests/assembler/parser_test.cpp)
target_include_directories(parser_test PRIVATE src)
target_compile_features(parser_test PRIVATE cxx_std_20)
add_test(NAME parser_test COMMAND parser_test)

add_executable(semantic_test tests/assembler/semantic_test.cpp)
target_include_directories(semantic_test PRIVATE src)
target_compile_features(semantic_test PRIVATE cxx_std_20)
add_test(NAME semantic_test COMMAND semantic_test)

add_executable(symbol_table_test tests/assembler/symbol_table_test.cpp)
target_include_directories(symbol_table_test PRIVATE src)
target_compile_features(symbol_table_test PRIVATE cxx_std_20)
add_test(NAME symbol_table_test COMMAND symbol_table_test)

add_executable(resolver_test tests/assembler/resolver_test.cpp)
target_include_directories(resolver_test PRIVATE src)
target_compile_features(resolver_test PRIVATE cxx_std_20)
add_test(NAME resolver_test COMMAND resolver_test)

add_executable(layout_test tests/assembler/layout_test.cpp)
target_include_directories(layout_test PRIVATE src)
add_test(NAME layout_test COMMAND layout_test)

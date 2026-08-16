cmake_minimum_required(VERSION 3.25)

project(TernaryMachine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ================================================================================================
# Common Interface
# ================================================================================================

add_library(ternary_machine INTERFACE)

target_include_directories(ternary_machine
    INTERFACE
        ${PROJECT_SOURCE_DIR}/src
)

# ================================================================================================
# Assembler Library
# ================================================================================================

add_library(ternary_machine_assembler STATIC
    src/assembler/assembler.cpp
)

target_link_libraries(ternary_machine_assembler
    PUBLIC
        ternary_machine
)

target_include_directories(ternary_machine_assembler
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

# ================================================================================================
# Runtime Library
# ================================================================================================

add_library(ternary_machine_runtime STATIC
    src/runtime/trn_loader.cpp
    src/runtime/image_loader.cpp
    src/runtime/tvm.cpp
)

target_link_libraries(ternary_machine_runtime PUBLIC ternary_machine)
target_include_directories(ternary_machine_runtime PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_compile_features(ternary_machine_runtime PUBLIC cxx_std_20)

# ================================================================================================
# Tools
# ================================================================================================

add_executable(tasm
    src/tools/tasm.cpp
)

target_link_libraries(tasm
    PRIVATE
        ternary_machine_assembler
)

add_executable(tvm
    src/tools/tvm.cpp
)

target_link_libraries(tvm PRIVATE ternary_machine_runtime)
target_include_directories(tvm PRIVATE ${PROJECT_SOURCE_DIR}/src)

# ================================================================================================
# Tests
# ================================================================================================

enable_testing()

# --------------------------------------------------------------------------------
# Ternary Tests
# --------------------------------------------------------------------------------

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

# --------------------------------------------------------------------------------
# VM Tests
# --------------------------------------------------------------------------------

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
target_link_libraries(vector_table_test PRIVATE ternary_machine)
add_test(NAME vector_table_test COMMAND vector_table_test)

# --------------------------------------------------------------------------------
# Assembler Tests
# --------------------------------------------------------------------------------

add_executable(lexer_test tests/assembler/lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE ternary_machine)
add_test(NAME lexer_test COMMAND lexer_test)

add_executable(parser_test tests/assembler/parser_test.cpp)
target_link_libraries(parser_test PRIVATE ternary_machine)
add_test(NAME parser_test COMMAND parser_test)

add_executable(semantic_test tests/assembler/semantic_test.cpp)
target_link_libraries(semantic_test PRIVATE ternary_machine)
add_test(NAME semantic_test COMMAND semantic_test)

add_executable(symbol_table_test tests/assembler/symbol_table_test.cpp)
target_link_libraries(symbol_table_test PRIVATE ternary_machine)
add_test(NAME symbol_table_test COMMAND symbol_table_test)

add_executable(resolver_test tests/assembler/resolver_test.cpp)
target_link_libraries(resolver_test PRIVATE ternary_machine)
add_test(NAME resolver_test COMMAND resolver_test)

add_executable(layout_test tests/assembler/layout_test.cpp)
target_link_libraries(layout_test PRIVATE ternary_machine)
add_test(NAME layout_test COMMAND layout_test)

add_executable(encoder_test tests/assembler/encoder_test.cpp)
target_link_libraries(encoder_test PRIVATE ternary_machine)
add_test(NAME encoder_test COMMAND encoder_test)

add_executable(trn_format_test tests/assembler/trn_format_test.cpp)
target_link_libraries(trn_format_test PRIVATE ternary_machine)
add_test(NAME trn_format_test COMMAND trn_format_test)

add_executable(trn_image_test tests/assembler/trn_image_test.cpp)
target_link_libraries(trn_image_test PRIVATE ternary_machine)
add_test(NAME trn_image_test COMMAND trn_image_test)

add_executable(trn_writer_test tests/assembler/trn_writer_test.cpp)
target_link_libraries(trn_writer_test PRIVATE ternary_machine)
add_test(NAME trn_writer_test COMMAND trn_writer_test)

add_executable(assembler_test tests/assembler/assembler_test.cpp)
target_link_libraries(assembler_test PRIVATE ternary_machine_assembler)
add_test(NAME assembler_test COMMAND assembler_test)

# --------------------------------------------------------------------------------
# Runtime Tests
# --------------------------------------------------------------------------------

add_executable(loaded_image_test tests/runtime/loaded_image_test.cpp)
target_link_libraries(loaded_image_test PRIVATE ternary_machine)
add_test(NAME loaded_image_test COMMAND loaded_image_test)

add_executable(trn_loader_test tests/runtime/trn_loader_test.cpp)
target_link_libraries(trn_loader_test PRIVATE
    ternary_machine_runtime
    ternary_machine_assembler
)
add_test(NAME trn_loader_test COMMAND trn_loader_test)

add_executable(image_loader_test tests/runtime/image_loader_test.cpp)
target_link_libraries(image_loader_test PRIVATE ternary_machine_runtime)
add_test(NAME image_loader_test COMMAND image_loader_test)

add_executable(execution_test tests/runtime/execution_test.cpp)
target_link_libraries(execution_test PRIVATE ternary_machine_runtime ternary_machine_assembler)
add_test(NAME execution_test COMMAND execution_test)

add_executable(control_flow_execution_test tests/runtime/control_flow_execution_test.cpp)
target_link_libraries(control_flow_execution_test PRIVATE ternary_machine_runtime ternary_machine_assembler)
add_test(NAME control_flow_execution_test COMMAND control_flow_execution_test)

add_executable(tvm_test tests/runtime/tvm_test.cpp)
target_link_libraries(tvm_test PRIVATE
    ternary_machine_runtime
    ternary_machine_assembler
)
add_test(NAME tvm_test COMMAND tvm_test)

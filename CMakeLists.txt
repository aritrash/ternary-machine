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

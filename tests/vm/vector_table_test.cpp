#include <cassert>
#include <cstdint>
#include "vm/vector_table.hpp"

using ternary_machine::vm::VectorTable;

int main() {
    static_assert(VectorTable::ResetAddress == 0);
    static_assert(VectorTable::VectorBase == 1);
    static_assert(VectorTable::SystemCallVector == 1);
    static_assert(VectorTable::InterruptVectorBase == 2);
    static_assert(VectorTable::ExceptionVectorBase == 29);
    static_assert(VectorTable::InterruptVectorCount == 27);
    static_assert(VectorTable::ExceptionVectorCount == 27);
    static_assert(VectorTable::VectorTableWords == 55);

    assert(VectorTable::valid_cause(0));
    assert(VectorTable::valid_cause(26));

    assert(VectorTable::interrupt_vector_address(0) == 2);
    assert(VectorTable::interrupt_vector_address(1) == 3);
    assert(VectorTable::interrupt_vector_address(26) == 28);

    assert(VectorTable::exception_vector_address(0) == 29);
    assert(VectorTable::exception_vector_address(1) == 30);
    assert(VectorTable::exception_vector_address(26) == 55);

    assert(VectorTable::system_call_vector().to_integer() == 1);
    assert(VectorTable::interrupt_vector(0).to_integer() == 2);
    assert(VectorTable::interrupt_vector(26).to_integer() == 28);
    assert(VectorTable::exception_vector(0).to_integer() == 29);
    assert(VectorTable::exception_vector(26).to_integer() == 55);

    return 0;
}

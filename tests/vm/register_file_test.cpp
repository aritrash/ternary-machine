#include <cassert>
#include <cstdint>

#include "vm/register_file.hpp"

using ternary_machine::ternary::Trit;
using ternary_machine::ternary::Word;
using ternary_machine::vm::Register;
using ternary_machine::vm::RegisterFile;

int main() {
    static_assert(RegisterFile::COUNT == 9);

    const RegisterFile initial;

    for (std::uint8_t i = 0; i < RegisterFile::COUNT; ++i) {
        const Register reg = static_cast<Register>(i);
        assert(initial.read(reg) == Word::zero());
    }

    RegisterFile registers;

    const Word value_a = Word::from_integer(123456);
    const Word value_b = Word::from_integer(-98765);

    registers.write(Register::R0, value_a);
    registers.write(Register::R8, value_b);

    assert(registers.read(Register::R0) == value_a);
    assert(registers.read(Register::R8) == value_b);

    for (std::uint8_t i = 1; i < 8; ++i)
        assert(registers.read(static_cast<Register>(i)) == Word::zero());

    registers.write(Register::R0, Word::from_integer(-1));

    assert(registers.read(Register::R0).to_integer() == -1);
    assert(registers.read(Register::R8).to_integer() == -98765);

    registers.clear();

    for (std::uint8_t i = 0; i < RegisterFile::COUNT; ++i)
        assert(registers.read(static_cast<Register>(i)) == Word::zero());

    return 0;
}

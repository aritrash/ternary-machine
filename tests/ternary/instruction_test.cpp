#include <cassert>
#include <cstdint>

#include "ternary/instruction.hpp"

using ternary_machine::ternary::Instruction;
using ternary_machine::ternary::Opcode;
using ternary_machine::ternary::Trit;

static Instruction::Cargo make_cargo(std::int64_t value) {
    Instruction::Cargo cargo{};

    for (std::size_t i = Instruction::CARGO_WIDTH; i-- > 0;) {
        const std::int64_t remainder = value % 3;

        if (remainder == 2) {
            cargo[i] = Trit::Neg;
            value = (value + 1) / 3;
        } else if (remainder == -2) {
            cargo[i] = Trit::Pos;
            value = (value - 1) / 3;
        } else {
            cargo[i] = static_cast<Trit>(remainder);
            value /= 3;
        }
    }

    assert(value == 0);
    return cargo;
}

int main() {
    static_assert(Instruction::WIDTH == 27);
    static_assert(Instruction::OPCODE_WIDTH == 3);
    static_assert(Instruction::RD_WIDTH == 2);
    static_assert(Instruction::RS1_WIDTH == 2);
    static_assert(Instruction::RS2_WIDTH == 2);
    static_assert(Instruction::CARGO_WIDTH == 18);

    const Instruction nop = Instruction::encode(Opcode::NOP, 0, 0, 0);

    assert(nop.opcode() == Opcode::NOP);
    assert(nop.rd() == 0);
    assert(nop.rs1() == 0);
    assert(nop.rs2() == 0);
    assert(nop.word().to_string() == "nnnnnnnnn000000000000000000");

    const Instruction add = Instruction::encode(Opcode::ADD, 3, 4, 5);

    assert(add.opcode() == Opcode::ADD);
    assert(add.rd() == 3);
    assert(add.rs1() == 4);
    assert(add.rs2() == 5);

    assert(add.word().trit(0) == Trit::Neg);
    assert(add.word().trit(1) == Trit::Neg);
    assert(add.word().trit(2) == Trit::Zero);

    const Instruction hlt = Instruction::encode(Opcode::HLT, 8, 7, 6);

    assert(hlt.opcode() == Opcode::HLT);
    assert(hlt.rd() == 8);
    assert(hlt.rs1() == 7);
    assert(hlt.rs2() == 6);

    for (std::int8_t opcode = -13; opcode <= 13; ++opcode) {
        const Opcode value = static_cast<Opcode>(opcode);
        const Instruction instruction = Instruction::encode(value, 0, 0, 0);
        assert(static_cast<std::int8_t>(instruction.opcode()) == opcode);
    }

    for (std::uint8_t reg = 0; reg < 9; ++reg) {
        const Instruction instruction = Instruction::encode(Opcode::ADD, reg, reg, reg);
        assert(instruction.rd() == reg);
        assert(instruction.rs1() == reg);
        assert(instruction.rs2() == reg);
    }

    Instruction::Cargo cargo{};
    cargo[0] = Trit::Neg;
    cargo[1] = Trit::Pos;
    cargo[17] = Trit::Pos;

    const Instruction cargo_instruction = Instruction::encode(Opcode::LDI, 2, 0, 0, cargo);
    const auto decoded_cargo = cargo_instruction.cargo();

    for (std::size_t i = 0; i < Instruction::CARGO_WIDTH; ++i)
        assert(decoded_cargo[i] == cargo[i]);

    assert(cargo_instruction.word().trit(9) == Trit::Neg);
    assert(cargo_instruction.word().trit(10) == Trit::Pos);
    assert(cargo_instruction.word().trit(26) == Trit::Pos);

    const Instruction original = Instruction::encode(Opcode::LDI, 4, 2, 7);
    const Instruction decoded = Instruction::decode(original.word());

    assert(decoded == original);
    assert(decoded.opcode() == Opcode::LDI);
    assert(decoded.rd() == 4);
    assert(decoded.rs1() == 2);
    assert(decoded.rs2() == 7);

    {
        const Instruction instruction = Instruction::encode(Opcode::LDI, 3, 0, 0, make_cargo(1));
        assert(instruction.immediate() == 1);
    }

    {
        const Instruction instruction = Instruction::encode(Opcode::LDI, 3, 0, 0, make_cargo(-1));
        assert(instruction.immediate() == -1);
    }

    {
        const Instruction instruction = Instruction::encode(Opcode::LDI, 3, 0, 0, make_cargo(123456));
        assert(instruction.immediate() == 123456);
    }

    {
        const Instruction instruction = Instruction::encode(Opcode::LDI, 3, 0, 0, make_cargo(-123456));
        assert(instruction.immediate() == -123456);
    }

    return 0;
}

#include <cassert>
#include <cstdint>

#include "ternary/instruction.hpp"

using ternary_machine::ternary::Instruction;
using ternary_machine::ternary::Opcode;
using ternary_machine::ternary::Trit;

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

    return 0;
}

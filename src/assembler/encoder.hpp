#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

#include "ir.hpp"
#include "ternary/instruction.hpp"

namespace ternary_machine::assembler {

class EncodingError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Encoder final {
public:
    [[nodiscard]] ternary::Instruction encode(const InstructionIR& instruction) const {
        const auto opcode = opcode_for(instruction.mnemonic);

        switch (opcode) {
            case ternary::Opcode::NOP:
            case ternary::Opcode::RET:
            case ternary::Opcode::SYS:
            case ternary::Opcode::IRET:
            case ternary::Opcode::SWAP:
            case ternary::Opcode::HLT:
                return encode_zero_operand(opcode, instruction);

            case ternary::Opcode::LDI:
                return encode_immediate(opcode, instruction);

            case ternary::Opcode::CMP:
                return encode_compare(opcode, instruction);

            case ternary::Opcode::TNOT:
            case ternary::Opcode::MOV:
                return encode_two_register(opcode, instruction);

            case ternary::Opcode::ADD:
            case ternary::Opcode::SUB:
            case ternary::Opcode::MUL:
            case ternary::Opcode::TAND:
            case ternary::Opcode::TOR:
            case ternary::Opcode::TXOR:
            case ternary::Opcode::SHF:
                return encode_three_register(opcode, instruction);

            case ternary::Opcode::LD:
            case ternary::Opcode::ST:
            case ternary::Opcode::LEA:
                return encode_memory(opcode, instruction);

            case ternary::Opcode::JMP:
            case ternary::Opcode::BEQ:
            case ternary::Opcode::BGT:
            case ternary::Opcode::BLT:
            case ternary::Opcode::CALL:
                return encode_branch(opcode, instruction);

            case ternary::Opcode::IN:
            case ternary::Opcode::OUT:
                return encode_io(opcode, instruction);
        }

        throw EncodingError("unsupported TVM opcode");
    }

private:
    [[nodiscard]] static ternary::Opcode opcode_for(std::string_view mnemonic) {
        if (mnemonic == "NOP") return ternary::Opcode::NOP;
        if (mnemonic == "ADD") return ternary::Opcode::ADD;
        if (mnemonic == "SUB") return ternary::Opcode::SUB;
        if (mnemonic == "MUL") return ternary::Opcode::MUL;
        if (mnemonic == "CMP") return ternary::Opcode::CMP;
        if (mnemonic == "TAND") return ternary::Opcode::TAND;
        if (mnemonic == "TOR") return ternary::Opcode::TOR;
        if (mnemonic == "TXOR") return ternary::Opcode::TXOR;
        if (mnemonic == "TNOT") return ternary::Opcode::TNOT;
        if (mnemonic == "SHF") return ternary::Opcode::SHF;
        if (mnemonic == "LDI") return ternary::Opcode::LDI;
        if (mnemonic == "MOV") return ternary::Opcode::MOV;
        if (mnemonic == "LD") return ternary::Opcode::LD;
        if (mnemonic == "ST") return ternary::Opcode::ST;
        if (mnemonic == "LEA") return ternary::Opcode::LEA;
        if (mnemonic == "JMP") return ternary::Opcode::JMP;
        if (mnemonic == "BEQ") return ternary::Opcode::BEQ;
        if (mnemonic == "BGT") return ternary::Opcode::BGT;
        if (mnemonic == "BLT") return ternary::Opcode::BLT;
        if (mnemonic == "CALL") return ternary::Opcode::CALL;
        if (mnemonic == "RET") return ternary::Opcode::RET;
        if (mnemonic == "IN") return ternary::Opcode::IN;
        if (mnemonic == "OUT") return ternary::Opcode::OUT;
        if (mnemonic == "SYS") return ternary::Opcode::SYS;
        if (mnemonic == "IRET") return ternary::Opcode::IRET;
        if (mnemonic == "SWAP") return ternary::Opcode::SWAP;
        if (mnemonic == "HLT") return ternary::Opcode::HLT;

        throw EncodingError("unknown TASM mnemonic: " + std::string(mnemonic));
    }

    [[nodiscard]] static std::uint8_t register_operand(const Operand& operand) {
        if (const auto* reg = std::get_if<RegisterOperand>(&operand))
            return reg->index;

        throw EncodingError("expected register operand");
    }

    [[nodiscard]] static std::int64_t immediate_operand(const Operand& operand) {
        if (const auto* immediate = std::get_if<ImmediateOperand>(&operand))
            return immediate->value;

        throw EncodingError("expected immediate operand");
    }

    [[nodiscard]] static ternary::Instruction::Cargo make_cargo(std::int64_t value) {
        ternary::Instruction::Cargo cargo{};

        for (std::size_t i = ternary::Instruction::CARGO_WIDTH; i-- > 0;) {
            const std::int64_t remainder = value % 3;

            if (remainder == 2) {
                cargo[i] = ternary::Trit::Neg;
                value = (value + 1) / 3;
            } else if (remainder == -2) {
                cargo[i] = ternary::Trit::Pos;
                value = (value - 1) / 3;
            } else {
                cargo[i] = static_cast<ternary::Trit>(remainder);
                value /= 3;
            }
        }

        if (value != 0)
            throw EncodingError("immediate does not fit in 18 trits");

        return cargo;
    }

    [[nodiscard]] static ternary::Instruction encode_zero_operand(ternary::Opcode opcode, const InstructionIR& instruction) {
        require_count(instruction, 0);
        return ternary::Instruction::encode(opcode, 0, 0, 0);
    }

    [[nodiscard]] static ternary::Instruction encode_immediate(ternary::Opcode opcode, const InstructionIR& instruction) {
        require_count(instruction, 2);

        const auto rd = register_operand(instruction.operands[0]);
        const auto immediate = immediate_operand(instruction.operands[1]);

        return ternary::Instruction::encode(opcode, rd, 0, 0, make_cargo(immediate));
    }

    [[nodiscard]] static ternary::Instruction encode_compare(ternary::Opcode opcode, const InstructionIR& instruction) {
        require_count(instruction, 2);

        const auto rs1 = register_operand(instruction.operands[0]);
        const auto rs2 = register_operand(instruction.operands[1]);

        return ternary::Instruction::encode(opcode, 0, rs1, rs2);
    }

    [[nodiscard]] static ternary::Instruction encode_two_register(ternary::Opcode opcode, const InstructionIR& instruction) {
        require_count(instruction, 2);

        const auto rd = register_operand(instruction.operands[0]);
        const auto rs1 = register_operand(instruction.operands[1]);

        return ternary::Instruction::encode(opcode, rd, rs1, 0);
    }

    [[nodiscard]] static ternary::Instruction encode_three_register(ternary::Opcode opcode, const InstructionIR& instruction) {
        require_count(instruction, 3);

        const auto rd = register_operand(instruction.operands[0]);
        const auto rs1 = register_operand(instruction.operands[1]);
        const auto rs2 = register_operand(instruction.operands[2]);

        return ternary::Instruction::encode(opcode, rd, rs1, rs2);
    }

    [[nodiscard]] static ternary::Instruction encode_memory(ternary::Opcode opcode, const InstructionIR& instruction) {
		require_count(instruction, 2);

		const auto* memory = std::get_if<MemoryOperand>(&instruction.operands[opcode == ternary::Opcode::ST ? 0 : 1]);

		if (!memory)
		    throw EncodingError("expected memory operand");

		if (opcode == ternary::Opcode::ST) {
		    const auto source = register_operand(instruction.operands[1]);
		    return ternary::Instruction::encode(opcode, 0, source, memory->base, make_cargo(memory->offset));
		}

		const auto rd = register_operand(instruction.operands[0]);
		return ternary::Instruction::encode(opcode, rd, memory->base, 0, make_cargo(memory->offset));
	}

    [[nodiscard]] static ternary::Instruction encode_branch(ternary::Opcode opcode, const InstructionIR& instruction) {
        require_count(instruction, 1);

        const auto immediate = immediate_operand(instruction.operands[0]);

        return ternary::Instruction::encode(opcode, 0, 0, 0, make_cargo(immediate));
    }

    [[nodiscard]] static ternary::Instruction encode_io(ternary::Opcode opcode, const InstructionIR& instruction) {
        require_count(instruction, 2);

        const auto reg = register_operand(instruction.operands[0]);
        const auto port = immediate_operand(instruction.operands[1]);

        return ternary::Instruction::encode(opcode, reg, 0, 0, make_cargo(port));
    }

    static void require_count(const InstructionIR& instruction, std::size_t expected) {
        if (instruction.operands.size() != expected)
            throw EncodingError("incorrect operand count for " + instruction.mnemonic);
    }
};

}

#include <cassert>
#include <initializer_list>
#include <vector>

#include "assembler/encoder.hpp"

using ternary_machine::assembler::Encoder;
using ternary_machine::assembler::EncodingError;
using ternary_machine::assembler::ImmediateOperand;
using ternary_machine::assembler::InstructionIR;
using ternary_machine::assembler::MemoryOperand;
using ternary_machine::assembler::Operand;
using ternary_machine::assembler::RegisterOperand;
using ternary_machine::ternary::Opcode;

static InstructionIR instruction(const char* mnemonic, std::initializer_list<Operand> operands = {}) {
    return InstructionIR{
        mnemonic,
        std::vector<Operand>(operands),
        {}
    };
}

static bool rejects(Encoder& encoder, const InstructionIR& instruction_ir) {
    try {
        static_cast<void>(encoder.encode(instruction_ir));
    } catch (const EncodingError&) {
        return true;
    }

    return false;
}

int main() {
    Encoder encoder;

    {
        const auto encoded = encoder.encode(instruction("NOP"));

        assert(encoded.opcode() == Opcode::NOP);
        assert(encoded.rd() == 0);
        assert(encoded.rs1() == 0);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == 0);
    }

    {
        const auto encoded = encoder.encode(instruction("HLT"));

        assert(encoded.opcode() == Opcode::HLT);
        assert(encoded.rd() == 0);
        assert(encoded.rs1() == 0);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == 0);
    }

    {
        const auto encoded = encoder.encode(instruction("RET"));

        assert(encoded.opcode() == Opcode::RET);
        assert(encoded.rd() == 0);
        assert(encoded.rs1() == 0);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == 0);
    }

    {
        const auto encoded = encoder.encode(instruction("SYS"));

        assert(encoded.opcode() == Opcode::SYS);
        assert(encoded.rd() == 0);
        assert(encoded.rs1() == 0);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == 0);
    }

    {
        const auto encoded = encoder.encode(instruction("IRET"));

        assert(encoded.opcode() == Opcode::IRET);
        assert(encoded.rd() == 0);
        assert(encoded.rs1() == 0);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == 0);
    }

    {
        const auto encoded = encoder.encode(instruction("SWAP"));

        assert(encoded.opcode() == Opcode::SWAP);
        assert(encoded.rd() == 0);
        assert(encoded.rs1() == 0);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == 0);
    }

    {
        const auto encoded = encoder.encode(instruction("ADD", {
            RegisterOperand{3},
            RegisterOperand{1},
            RegisterOperand{2}
        }));

        assert(encoded.opcode() == Opcode::ADD);
        assert(encoded.rd() == 3);
        assert(encoded.rs1() == 1);
        assert(encoded.rs2() == 2);
        assert(encoded.immediate() == 0);
    }

    {
        const auto encoded = encoder.encode(instruction("SUB", {
            RegisterOperand{4},
            RegisterOperand{2},
            RegisterOperand{1}
        }));

        assert(encoded.opcode() == Opcode::SUB);
        assert(encoded.rd() == 4);
        assert(encoded.rs1() == 2);
        assert(encoded.rs2() == 1);
    }

    {
        const auto encoded = encoder.encode(instruction("MUL", {
            RegisterOperand{5},
            RegisterOperand{3},
            RegisterOperand{7}
        }));

        assert(encoded.opcode() == Opcode::MUL);
        assert(encoded.rd() == 5);
        assert(encoded.rs1() == 3);
        assert(encoded.rs2() == 7);
    }

    {
        const auto encoded = encoder.encode(instruction("TAND", {
            RegisterOperand{1},
            RegisterOperand{2},
            RegisterOperand{3}
        }));

        assert(encoded.opcode() == Opcode::TAND);
        assert(encoded.rd() == 1);
        assert(encoded.rs1() == 2);
        assert(encoded.rs2() == 3);
    }

    {
        const auto encoded = encoder.encode(instruction("TOR", {
            RegisterOperand{1},
            RegisterOperand{2},
            RegisterOperand{3}
        }));

        assert(encoded.opcode() == Opcode::TOR);
        assert(encoded.rd() == 1);
        assert(encoded.rs1() == 2);
        assert(encoded.rs2() == 3);
    }

    {
        const auto encoded = encoder.encode(instruction("TXOR", {
            RegisterOperand{1},
            RegisterOperand{2},
            RegisterOperand{3}
        }));

        assert(encoded.opcode() == Opcode::TXOR);
        assert(encoded.rd() == 1);
        assert(encoded.rs1() == 2);
        assert(encoded.rs2() == 3);
    }

    {
        const auto encoded = encoder.encode(instruction("SHF", {
            RegisterOperand{1},
            RegisterOperand{2},
            RegisterOperand{3}
        }));

        assert(encoded.opcode() == Opcode::SHF);
        assert(encoded.rd() == 1);
        assert(encoded.rs1() == 2);
        assert(encoded.rs2() == 3);
        assert(encoded.immediate() == 0);
    }

    {
        const auto encoded = encoder.encode(instruction("CMP", {
            RegisterOperand{2},
            RegisterOperand{7}
        }));

        assert(encoded.opcode() == Opcode::CMP);
        assert(encoded.rd() == 0);
        assert(encoded.rs1() == 2);
        assert(encoded.rs2() == 7);
    }

    {
        const auto encoded = encoder.encode(instruction("TNOT", {
            RegisterOperand{4},
            RegisterOperand{2}
        }));

        assert(encoded.opcode() == Opcode::TNOT);
        assert(encoded.rd() == 4);
        assert(encoded.rs1() == 2);
        assert(encoded.rs2() == 0);
    }

    {
        const auto encoded = encoder.encode(instruction("MOV", {
            RegisterOperand{4},
            RegisterOperand{2}
        }));

        assert(encoded.opcode() == Opcode::MOV);
        assert(encoded.rd() == 4);
        assert(encoded.rs1() == 2);
        assert(encoded.rs2() == 0);
    }

    {
        const auto encoded = encoder.encode(instruction("LDI", {
            RegisterOperand{1},
            ImmediateOperand{42}
        }));

        assert(encoded.opcode() == Opcode::LDI);
        assert(encoded.rd() == 1);
        assert(encoded.rs1() == 0);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == 42);
    }

    {
        const auto encoded = encoder.encode(instruction("LDI", {
            RegisterOperand{8},
            ImmediateOperand{-123456}
        }));

        assert(encoded.opcode() == Opcode::LDI);
        assert(encoded.rd() == 8);
        assert(encoded.immediate() == -123456);
    }

    {
        const auto encoded = encoder.encode(instruction("LD", {
            RegisterOperand{1},
            MemoryOperand{2, 10}
        }));

        assert(encoded.opcode() == Opcode::LD);
        assert(encoded.rd() == 1);
        assert(encoded.rs1() == 2);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == 10);
    }

    {
        const auto encoded = encoder.encode(instruction("LD", {
            RegisterOperand{7},
            MemoryOperand{4, -27}
        }));

        assert(encoded.opcode() == Opcode::LD);
        assert(encoded.rd() == 7);
        assert(encoded.rs1() == 4);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == -27);
    }

    {
        const auto encoded = encoder.encode(instruction("LEA", {
            RegisterOperand{3},
            MemoryOperand{5, 27}
        }));

        assert(encoded.opcode() == Opcode::LEA);
        assert(encoded.rd() == 3);
        assert(encoded.rs1() == 5);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == 27);
    }

    {
        const auto encoded = encoder.encode(instruction("ST", {
            MemoryOperand{2, 10},
            RegisterOperand{1}
        }));

        assert(encoded.opcode() == Opcode::ST);
        assert(encoded.rd() == 0);
        assert(encoded.rs1() == 1);
        assert(encoded.rs2() == 2);
        assert(encoded.immediate() == 10);
    }

    {
        const auto encoded = encoder.encode(instruction("ST", {
            MemoryOperand{2, -10},
            RegisterOperand{1}
        }));

        assert(encoded.opcode() == Opcode::ST);
        assert(encoded.rd() == 0);
        assert(encoded.rs1() == 1);
        assert(encoded.rs2() == 2);
        assert(encoded.immediate() == -10);
    }

    {
        const auto encoded = encoder.encode(instruction("JMP", {
            ImmediateOperand{7}
        }));

        assert(encoded.opcode() == Opcode::JMP);
        assert(encoded.rd() == 0);
        assert(encoded.rs1() == 0);
        assert(encoded.rs2() == 0);
        assert(encoded.immediate() == 7);
    }

    {
        const auto encoded = encoder.encode(instruction("BEQ", {
            ImmediateOperand{-5}
        }));

        assert(encoded.opcode() == Opcode::BEQ);
        assert(encoded.immediate() == -5);
    }

    {
        const auto encoded = encoder.encode(instruction("BGT", {
            ImmediateOperand{12}
        }));

        assert(encoded.opcode() == Opcode::BGT);
        assert(encoded.immediate() == 12);
    }

    {
        const auto encoded = encoder.encode(instruction("BLT", {
            ImmediateOperand{-12}
        }));

        assert(encoded.opcode() == Opcode::BLT);
        assert(encoded.immediate() == -12);
    }

    {
        const auto encoded = encoder.encode(instruction("CALL", {
            ImmediateOperand{-4}
        }));

        assert(encoded.opcode() == Opcode::CALL);
        assert(encoded.immediate() == -4);
    }

    {
        const auto encoded = encoder.encode(instruction("IN", {
            RegisterOperand{2},
            ImmediateOperand{5}
        }));

        assert(encoded.opcode() == Opcode::IN);
        assert(encoded.rd() == 2);
        assert(encoded.immediate() == 5);
    }

    {
        const auto encoded = encoder.encode(instruction("OUT", {
            RegisterOperand{6},
            ImmediateOperand{3}
        }));

        assert(encoded.opcode() == Opcode::OUT);
        assert(encoded.rd() == 6);
        assert(encoded.immediate() == 3);
    }

    {
        assert(rejects(encoder, instruction("ADD", {
            RegisterOperand{1},
            RegisterOperand{2}
        })));
    }

    {
        assert(rejects(encoder, instruction("ADD", {
            RegisterOperand{1},
            RegisterOperand{2},
            RegisterOperand{3},
            RegisterOperand{4}
        })));
    }

    {
        assert(rejects(encoder, instruction("HLT", {
            RegisterOperand{1}
        })));
    }

    {
        assert(rejects(encoder, instruction("LDI", {
            RegisterOperand{1}
        })));
    }

    {
        assert(rejects(encoder, instruction("LDI", {
            RegisterOperand{1},
            RegisterOperand{2}
        })));
    }

    {
        assert(rejects(encoder, instruction("ADD", {
            RegisterOperand{1},
            ImmediateOperand{10},
            RegisterOperand{3}
        })));
    }

    {
        assert(rejects(encoder, instruction("SHF", {
            RegisterOperand{1},
            RegisterOperand{2},
            ImmediateOperand{3}
        })));
    }

    {
        assert(rejects(encoder, instruction("CMP", {
            RegisterOperand{1},
            ImmediateOperand{2}
        })));
    }

    {
        assert(rejects(encoder, instruction("MOV", {
            RegisterOperand{1},
            ImmediateOperand{2}
        })));
    }

    {
        assert(rejects(encoder, instruction("LD", {
            RegisterOperand{1},
            RegisterOperand{2}
        })));
    }

    {
        assert(rejects(encoder, instruction("LD", {
            RegisterOperand{1},
            MemoryOperand{2, 10},
            RegisterOperand{3}
        })));
    }

    {
        assert(rejects(encoder, instruction("ST", {
            RegisterOperand{1},
            MemoryOperand{2, 10}
        })));
    }

    {
        assert(rejects(encoder, instruction("ST", {
            MemoryOperand{2, 10},
            ImmediateOperand{1}
        })));
    }

    {
        assert(rejects(encoder, instruction("LEA", {
            RegisterOperand{1},
            RegisterOperand{2}
        })));
    }

    {
        assert(rejects(encoder, instruction("JMP", {
            RegisterOperand{1}
        })));
    }

    {
        assert(rejects(encoder, instruction("CALL", {
            RegisterOperand{1}
        })));
    }

    {
        assert(rejects(encoder, instruction("IN", {
            ImmediateOperand{1},
            ImmediateOperand{2}
        })));
    }

    {
        assert(rejects(encoder, instruction("OUT", {
            ImmediateOperand{1},
            ImmediateOperand{2}
        })));
    }

    {
        assert(rejects(encoder, instruction("MEOW")));
    }

    {
        assert(rejects(encoder, instruction("LDI", {
            RegisterOperand{1},
            ImmediateOperand{1LL << 60}
        })));
    }

    {
        assert(rejects(encoder, instruction("LDI", {
            RegisterOperand{1},
            ImmediateOperand{-(1LL << 60)}
        })));
    }

    return 0;
}

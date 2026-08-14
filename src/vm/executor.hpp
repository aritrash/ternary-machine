#pragma once

#include <stdexcept>

#include "machine.hpp"
#include "ternary/instruction.hpp"
#include "alu.hpp"

namespace ternary_machine::vm {

class ExecutionError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Executor final {
public:
    void step(Machine& machine) const {
        if (machine.halted())
            throw ExecutionError("machine is halted");

        const auto pc = machine.cpu().pc();
        const auto word = machine.memory().read(pc);
        const auto instruction = ternary::Instruction::decode(word);

        switch (instruction.opcode()) {
            case ternary::Opcode::NOP:
                machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                return;

            case ternary::Opcode::HLT:
                machine.halt();
                return;

            case ternary::Opcode::LDI:
                machine.cpu().registers().write(
                    static_cast<Register>(instruction.rd()),
                    ternary::Word::from_integer(instruction.immediate())
                );
                machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                return;
                
            case ternary::Opcode::MOV:
				machine.cpu().registers().write(
					static_cast<Register>(instruction.rd()),
					machine.cpu().registers().read(static_cast<Register>(instruction.rs1()))
				);
				machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
				return;
				
			case ternary::Opcode::ADD:
				machine.cpu().registers().write(
					static_cast<Register>(instruction.rd()),
					ALU::add(
						machine.cpu().registers().read(static_cast<Register>(instruction.rs1())),
						machine.cpu().registers().read(static_cast<Register>(instruction.rs2()))
					)
				);
				machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
				return;

			case ternary::Opcode::SUB:
				machine.cpu().registers().write(
					static_cast<Register>(instruction.rd()),
					ALU::sub(
						machine.cpu().registers().read(static_cast<Register>(instruction.rs1())),
						machine.cpu().registers().read(static_cast<Register>(instruction.rs2()))
					)
				);
				machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
				return;

			case ternary::Opcode::MUL:
				machine.cpu().registers().write(
					static_cast<Register>(instruction.rd()),
					ALU::mul(
						machine.cpu().registers().read(static_cast<Register>(instruction.rs1())),
						machine.cpu().registers().read(static_cast<Register>(instruction.rs2()))
					)
				);
				machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
				return;
				
		    case ternary::Opcode::TAND:
                machine.cpu().registers().write(
                    static_cast<Register>(instruction.rd()),
                    ALU::tand(
                        machine.cpu().registers().read(static_cast<Register>(instruction.rs1())),
                        machine.cpu().registers().read(static_cast<Register>(instruction.rs2()))
                    )
                );
                machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                return;

            case ternary::Opcode::TOR:
                machine.cpu().registers().write(
                    static_cast<Register>(instruction.rd()),
                    ALU::tor(
                        machine.cpu().registers().read(static_cast<Register>(instruction.rs1())),
                        machine.cpu().registers().read(static_cast<Register>(instruction.rs2()))
                    )
                );
                machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                return;

            case ternary::Opcode::TXOR:
                machine.cpu().registers().write(
                    static_cast<Register>(instruction.rd()),
                    ALU::txor(
                        machine.cpu().registers().read(static_cast<Register>(instruction.rs1())),
                        machine.cpu().registers().read(static_cast<Register>(instruction.rs2()))
                    )
                );
                machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                return;

            case ternary::Opcode::TNOT:
                machine.cpu().registers().write(
                    static_cast<Register>(instruction.rd()),
                    ALU::tnot(
                        machine.cpu().registers().read(static_cast<Register>(instruction.rs1()))
                    )
                );
                machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                return;
                
            case ternary::Opcode::CMP:
				machine.cpu().set_status(
					ALU::compare(
						machine.cpu().registers().read(static_cast<Register>(instruction.rs1())),
						machine.cpu().registers().read(static_cast<Register>(instruction.rs2()))
					)
				);
				machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
				return;

            default:
                throw ExecutionError("unsupported TVM opcode");
        }
    }
};

}

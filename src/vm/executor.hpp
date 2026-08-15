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
                
            case ternary::Opcode::SHF:
                machine.cpu().registers().write(
                    static_cast<Register>(instruction.rd()),
                    ALU::shift(
                        machine.cpu().registers().read(static_cast<Register>(instruction.rs1())),
                        machine.cpu().registers().read(static_cast<Register>(instruction.rs2())).to_integer()
                    )
                );
                machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                return;
                
            case ternary::Opcode::LD: {
                const auto base = machine.cpu().registers().read(static_cast<Register>(instruction.rs1()));
                const auto offset = ternary::Word::from_integer(instruction.immediate());
                const auto address = ALU::add(base, offset);

                machine.cpu().registers().write(
                    static_cast<Register>(instruction.rd()),
                    machine.memory().read(address)
                );

                machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                return;
            }

            case ternary::Opcode::ST: {
                const auto data = machine.cpu().registers().read(static_cast<Register>(instruction.rs1()));
                const auto base = machine.cpu().registers().read(static_cast<Register>(instruction.rs2()));
                const auto offset = ternary::Word::from_integer(instruction.immediate());
                const auto address = ALU::add(base, offset);

                machine.memory().write(address, data);

                machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                return;
            }

            case ternary::Opcode::LEA: {
                const auto base = machine.cpu().registers().read(static_cast<Register>(instruction.rs1()));
                const auto offset = ternary::Word::from_integer(instruction.immediate());

                machine.cpu().registers().write(
                    static_cast<Register>(instruction.rd()),
                    ALU::add(base, offset)
                );

                machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                return;
            }
            
            case ternary::Opcode::JMP: {
                const auto offset = ternary::Word::from_integer(instruction.immediate());
                machine.cpu().set_pc(ALU::add(pc, offset));
                return;
            }

            case ternary::Opcode::BEQ: {
                if (machine.cpu().status() == Comparison::Equal) {
                    const auto offset = ternary::Word::from_integer(instruction.immediate());
                    machine.cpu().set_pc(ALU::add(pc, offset));
                } else {
                    machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                }
                return;
            }

            case ternary::Opcode::BGT: {
                if (machine.cpu().status() == Comparison::Greater) {
                    const auto offset = ternary::Word::from_integer(instruction.immediate());
                    machine.cpu().set_pc(ALU::add(pc, offset));
                } else {
                    machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                }
                return;
            }

            case ternary::Opcode::BLT: {
                if (machine.cpu().status() == Comparison::Less) {
                    const auto offset = ternary::Word::from_integer(instruction.immediate());
                    machine.cpu().set_pc(ALU::add(pc, offset));
                } else {
                    machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
                }
                return;
            }
            
            case ternary::Opcode::CALL: {
                const auto return_address = ternary::Word::from_integer(pc.to_integer() + 1);
                const auto offset = ternary::Word::from_integer(instruction.immediate());
                const auto new_sp = ternary::Word::from_integer(machine.cpu().sp().to_integer() - 1);

                machine.cpu().set_sp(new_sp);
                machine.memory().write(new_sp, return_address);
                machine.cpu().set_pc(ALU::add(pc, offset));
                return;
            }

            case ternary::Opcode::RET: {
                const auto sp = machine.cpu().sp();
                const auto return_address = machine.memory().read(sp);

                machine.cpu().set_pc(return_address);
                machine.cpu().set_sp(ternary::Word::from_integer(sp.to_integer() + 1));
                return;
            }
                
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

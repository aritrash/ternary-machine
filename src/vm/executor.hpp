#pragma once

#include <stdexcept>

#include "machine.hpp"
#include "ternary/instruction.hpp"

namespace ternary_machine::vm {

	class ExecutionError : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	class Executor final {
	public:
		void step(Machine& machine) const {
		    const auto pc = machine.cpu().pc();
		    const auto word = machine.memory().read(pc);
		    const auto instruction = ternary::Instruction::decode(word);

		    switch (instruction.opcode()) {
		        case ternary::Opcode::NOP:
		            machine.cpu().set_pc(ternary::Word::from_integer(pc.to_integer() + 1));
		            return;

		        default:
		            throw ExecutionError("unsupported TVM opcode");
		    }
		}
	};
}

#include <cassert>

#include "vm/executor.hpp"
#include "ternary/instruction.hpp"

using ternary_machine::ternary::Instruction;
using ternary_machine::ternary::Opcode;
using ternary_machine::ternary::Word;
using ternary_machine::vm::ExecutionError;
using ternary_machine::vm::Executor;
using ternary_machine::vm::Machine;

int main() {
    Executor executor;

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::NOP, 0, 0, 0);

        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        assert(machine.cpu().pc() == address);

        executor.step(machine);

        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(42);
        const Instruction instruction = Instruction::encode(Opcode::NOP, 0, 0, 0);

        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().pc() == Word::from_integer(43));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::ADD, 0, 1, 2);

        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        bool threw = false;

        try {
            executor.step(machine);
        } catch (const ExecutionError&) {
            threw = true;
        }

        assert(threw);
        assert(machine.cpu().pc() == address);
    }

    return 0;
}

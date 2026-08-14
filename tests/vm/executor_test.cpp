#include <cassert>

#include "vm/executor.hpp"
#include "ternary/instruction.hpp"

using ternary_machine::ternary::Instruction;
using ternary_machine::ternary::Opcode;
using ternary_machine::ternary::Word;
using ternary_machine::ternary::Trit;
using ternary_machine::vm::ExecutionError;
using ternary_machine::vm::Executor;
using ternary_machine::vm::Machine;
using ternary_machine::vm::Register;

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
    Executor executor;

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::NOP, 0, 0, 0);

        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        assert(!machine.halted());

        executor.step(machine);

        assert(!machine.halted());
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(42);
        const Instruction instruction = Instruction::encode(Opcode::NOP, 0, 0, 0);

        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(!machine.halted());
        assert(machine.cpu().pc() == Word::from_integer(43));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::HLT, 0, 0, 0);

        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        assert(!machine.halted());

        executor.step(machine);

        assert(machine.halted());
        assert(machine.cpu().pc() == address);
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::HLT, 0, 0, 0);

        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.halted());

        bool threw = false;

        try {
            executor.step(machine);
        } catch (const ExecutionError&) {
            threw = true;
        }

        assert(threw);
        assert(machine.halted());
        assert(machine.cpu().pc() == address);
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
        assert(!machine.halted());
        assert(machine.cpu().pc() == address);
    }

    {
        Machine machine;
        machine.halt();

        assert(machine.halted());

        machine.reset();

        assert(!machine.halted());
        assert(machine.cpu().pc() == Word::zero());
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::LDI, 3, 0, 0, make_cargo(1));

        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        assert(machine.cpu().registers().read(Register::R3) == Word::zero());

        executor.step(machine);

        assert(!machine.halted());
        assert(machine.cpu().registers().read(Register::R3).to_integer() == 1);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::LDI, 7, 0, 0, make_cargo(-1));

        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R7).to_integer() == -1);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::LDI, 5, 0, 0, make_cargo(123456));

        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R5).to_integer() == 123456);
        assert(machine.cpu().registers().read(Register::R4) == Word::zero());
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    return 0;
}

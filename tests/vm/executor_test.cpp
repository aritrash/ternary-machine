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
using ternary_machine::vm::Comparison;

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

	// NOP Tests
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
    
	// HLT tests
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
    
	// Arithmetic Tests
    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::ADD, 3, 1, 2);

        machine.cpu().registers().write(Register::R1, Word::from_integer(5));
        machine.cpu().registers().write(Register::R2, Word::from_integer(7));
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(!machine.halted());
        assert(machine.cpu().registers().read(Register::R3).to_integer() == 12);
        assert(machine.cpu().registers().read(Register::R1).to_integer() == 5);
        assert(machine.cpu().registers().read(Register::R2).to_integer() == 7);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }
    
        {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::SUB, 3, 1, 2);

        machine.cpu().registers().write(Register::R1, Word::from_integer(5));
        machine.cpu().registers().write(Register::R2, Word::from_integer(7));
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R3).to_integer() == -2);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::MUL, 3, 1, 2);

        machine.cpu().registers().write(Register::R1, Word::from_integer(5));
        machine.cpu().registers().write(Register::R2, Word::from_integer(7));
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R3).to_integer() == 35);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::CMP, 0, 1, 2);

        machine.cpu().registers().write(Register::R1, Word::from_integer(5));
        machine.cpu().registers().write(Register::R2, Word::from_integer(7));
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().status() == Comparison::Less);
        assert(machine.cpu().registers().read(Register::R0) == Word::zero());
        assert(machine.cpu().pc() == Word::from_integer(1));
    }
    
    // Logic Tests
    {
        Machine machine;
        const Word address = Word::from_integer(0);

        machine.cpu().registers().write(Register::R1, Word::from_integer(-1));
		machine.cpu().registers().write(Register::R2, Word::from_integer(1));

        const Instruction instruction = Instruction::encode(Opcode::TAND, 3, 1, 2);
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R3).to_integer() == -1);
        assert(machine.cpu().registers().read(Register::R1).to_integer() == -1);
        assert(machine.cpu().registers().read(Register::R2).to_integer() == 1);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);

        machine.cpu().registers().write(Register::R1, Word::from_integer(-1));
        machine.cpu().registers().write(Register::R2, Word::from_integer(1));

        const Instruction instruction = Instruction::encode(Opcode::TOR, 3, 1, 2);
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R3).to_integer() == 1);
        assert(machine.cpu().registers().read(Register::R1).to_integer() == -1);
        assert(machine.cpu().registers().read(Register::R2).to_integer() == 1);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);

        machine.cpu().registers().write(Register::R1, Word::from_integer(-1));
        machine.cpu().registers().write(Register::R2, Word::from_integer(1));

        const Instruction instruction = Instruction::encode(Opcode::TXOR, 3, 1, 2);
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R3).to_integer() == 1);
        assert(machine.cpu().registers().read(Register::R1).to_integer() == -1);
        assert(machine.cpu().registers().read(Register::R2).to_integer() == 1);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);

        machine.cpu().registers().write(Register::R1, Word::from_integer(-1));

        const Instruction instruction = Instruction::encode(Opcode::TNOT, 3, 1, 0);
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R3).to_integer() == 1);
        assert(machine.cpu().registers().read(Register::R1).to_integer() == -1);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }
    
    // SHF Tests
    {
        Machine machine;
        const Word address = Word::from_integer(0);

        const Word value = Word::from_integer(1);
        const Word shift = Word::from_integer(3);

        machine.cpu().registers().write(Register::R1, value);
        machine.cpu().registers().write(Register::R2, shift);

        const Instruction instruction = Instruction::encode(Opcode::SHF, 3, 1, 2);
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R3).to_integer() == 27);
        assert(machine.cpu().registers().read(Register::R1) == value);
        assert(machine.cpu().registers().read(Register::R2) == shift);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);

        const Word value = Word::from_integer(2187);
        const Word shift = Word::from_integer(-3);

        machine.cpu().registers().write(Register::R1, value);
        machine.cpu().registers().write(Register::R2, shift);

        const Instruction instruction = Instruction::encode(Opcode::SHF, 3, 1, 2);
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R3).to_integer() == 81);
        assert(machine.cpu().registers().read(Register::R1) == value);
        assert(machine.cpu().registers().read(Register::R2) == shift);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }
    
    // LD - Zero Offset
    {
        Machine machine;
        const Word address = Word::from_integer(100);
        const Word value = Word::from_integer(12345);

        machine.cpu().registers().write(Register::R1, address);
        machine.memory().write(address, value);

        const Instruction instruction = Instruction::encode(Opcode::LD, 2, 1, 0);
        machine.memory().write(Word::zero(), instruction.word());
        machine.cpu().set_pc(Word::zero());

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R2) == value);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }
    
    // LD - Positive & Negative Offset
    {
        Machine machine;
        const Word base = Word::from_integer(100);
        const Word value = Word::from_integer(54321);

        machine.cpu().registers().write(Register::R1, base);
        machine.memory().write(Word::from_integer(105), value);

        const Instruction instruction = Instruction::encode(Opcode::LD, 2, 1, 0, make_cargo(5));
        machine.memory().write(Word::zero(), instruction.word());
        machine.cpu().set_pc(Word::zero());

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R2) == value);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word base = Word::from_integer(100);
        const Word value = Word::from_integer(-54321);

        machine.cpu().registers().write(Register::R1, base);
        machine.memory().write(Word::from_integer(95), value);

        const Instruction instruction = Instruction::encode(Opcode::LD, 2, 1, 0, make_cargo(-5));
        machine.memory().write(Word::zero(), instruction.word());
        machine.cpu().set_pc(Word::zero());

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R2) == value);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }
    
    // ST - Positive & Negative Offset
    {
        Machine machine;
        const Word base = Word::from_integer(200);
        const Word value = Word::from_integer(7777);

        machine.cpu().registers().write(Register::R1, value);
        machine.cpu().registers().write(Register::R2, base);

        const Instruction instruction = Instruction::encode(Opcode::ST, 0, 1, 2, make_cargo(7));
        machine.memory().write(Word::zero(), instruction.word());
        machine.cpu().set_pc(Word::zero());

        executor.step(machine);

        assert(machine.memory().read(Word::from_integer(207)) == value);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }
    
    {
        Machine machine;
        const Word base = Word::from_integer(200);
        const Word value = Word::from_integer(-7777);

        machine.cpu().registers().write(Register::R1, value);
        machine.cpu().registers().write(Register::R2, base);

        const Instruction instruction = Instruction::encode(Opcode::ST, 0, 1, 2, make_cargo(-7));
        machine.memory().write(Word::zero(), instruction.word());
        machine.cpu().set_pc(Word::zero());

        executor.step(machine);

        assert(machine.memory().read(Word::from_integer(193)) == value);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }
    
    // LEA - Positive Offsets
    {
        Machine machine;
        const Word base = Word::from_integer(500);

        machine.cpu().registers().write(Register::R1, base);

        const Instruction instruction = Instruction::encode(Opcode::LEA, 2, 1, 0, make_cargo(25));
        machine.memory().write(Word::zero(), instruction.word());
        machine.cpu().set_pc(Word::zero());

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R2) == Word::from_integer(525));
        assert(machine.memory().size() == 1);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }
    
    // LEA - Negative offset and no memory access
    {
        Machine machine;
        const Word base = Word::from_integer(500);

        machine.cpu().registers().write(Register::R1, base);

        const Instruction instruction = Instruction::encode(Opcode::LEA, 2, 1, 0, make_cargo(-25));
        machine.memory().write(Word::zero(), instruction.word());
        machine.cpu().set_pc(Word::zero());

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R2) == Word::from_integer(475));
        assert(machine.memory().read(Word::from_integer(475)) == Word::zero());
        assert(machine.cpu().pc() == Word::from_integer(1));
    }
    
    {
        Machine machine;

        machine.cpu().registers().write(Register::R1, Word::from_integer(1234));
        machine.cpu().registers().write(Register::R2, Word::from_integer(300));

        const Instruction instruction = Instruction::encode(Opcode::ST, 8, 1, 2, make_cargo(10));
        machine.memory().write(Word::zero(), instruction.word());
        machine.cpu().set_pc(Word::zero());

        executor.step(machine);

        assert(machine.memory().read(Word::from_integer(310)) == Word::from_integer(1234));
        assert(machine.cpu().registers().read(Register::R8) == Word::zero());
    }
    
    
    {
        Machine machine;
        const Word address = Word::from_integer(0);

        const Word lhs = Word::from_integer(-6561);
        const Word rhs = Word::from_integer(2187);

        machine.cpu().registers().write(Register::R1, lhs);
        machine.cpu().registers().write(Register::R2, rhs);

        const Instruction instruction = Instruction::encode(Opcode::TAND, 3, 1, 2);
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        const Word result = machine.cpu().registers().read(Register::R3);

        for (std::size_t i = 0; i < Word::WIDTH; ++i) {
            const auto a = lhs.trit(i);
            const auto b = rhs.trit(i);
            assert(result.trit(i) == (static_cast<int>(a) < static_cast<int>(b) ? a : b));
        }

        assert(machine.cpu().pc() == Word::from_integer(1));
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
    
    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::MOV, 3, 1, 0);

        machine.cpu().registers().write(Register::R1, Word::from_integer(12345));
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        assert(machine.cpu().registers().read(Register::R3) == Word::zero());

        executor.step(machine);

        assert(!machine.halted());
        assert(machine.cpu().registers().read(Register::R3).to_integer() == 12345);
        assert(machine.cpu().registers().read(Register::R1).to_integer() == 12345);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::MOV, 7, 2, 0);

        machine.cpu().registers().write(Register::R2, Word::from_integer(-9876));
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R7).to_integer() == -9876);
        assert(machine.cpu().registers().read(Register::R2).to_integer() == -9876);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        Machine machine;
        const Word address = Word::from_integer(0);
        const Instruction instruction = Instruction::encode(Opcode::MOV, 4, 4, 0);

        machine.cpu().registers().write(Register::R4, Word::from_integer(456789));
        machine.memory().write(address, instruction.word());
        machine.cpu().set_pc(address);

        executor.step(machine);

        assert(machine.cpu().registers().read(Register::R4).to_integer() == 456789);
        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    return 0;
}

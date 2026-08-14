#include <cassert>

#include "vm/cpu_state.hpp"

using ternary_machine::ternary::Word;
using ternary_machine::vm::Comparison;
using ternary_machine::vm::CPUState;
using ternary_machine::vm::Register;

int main() {
    CPUState state;

    assert(state.pc() == Word::zero());
    assert(state.sp() == Word::zero());
    assert(state.status() == Comparison::Equal);

    for (int i = 0; i < 9; ++i)
        assert(state.registers().read(static_cast<Register>(i)) == Word::zero());

    const Word pc = Word::from_integer(12345);
    const Word sp = Word::from_integer(987654);

    state.set_pc(pc);
    state.set_sp(sp);
    state.set_status(Comparison::Greater);

    state.registers().write(Register::R1, Word::from_integer(42));
    state.registers().write(Register::R7, Word::from_integer(-42));

    assert(state.pc() == pc);
    assert(state.sp() == sp);
    assert(state.status() == Comparison::Greater);
    assert(state.registers().read(Register::R1).to_integer() == 42);
    assert(state.registers().read(Register::R7).to_integer() == -42);

    state.set_status(Comparison::Less);
    assert(state.status() == Comparison::Less);

    state.set_status(Comparison::Equal);
    assert(state.status() == Comparison::Equal);

    state.reset();

    assert(state.pc() == Word::zero());
    assert(state.sp() == Word::zero());
    assert(state.status() == Comparison::Equal);

    for (int i = 0; i < 9; ++i)
        assert(state.registers().read(static_cast<Register>(i)) == Word::zero());

    return 0;
}

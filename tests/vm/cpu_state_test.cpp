#include <cassert>

#include "vm/cpu_state.hpp"

using ternary_machine::ternary::Word;
using ternary_machine::vm::Comparison;
using ternary_machine::vm::CPUState;
using ternary_machine::vm::PrivilegeLevel;
using ternary_machine::vm::Register;
using ternary_machine::vm::TransitionCause;

int main() {
    static_assert(static_cast<std::int8_t>(PrivilegeLevel::User) == 0);
    static_assert(static_cast<std::int8_t>(PrivilegeLevel::Kernel) == 1);

    CPUState state;

    assert(state.pc() == Word::zero());
    assert(state.sp() == Word::zero());
    assert(state.ksp() == Word::zero());
    assert(state.status() == Comparison::Equal);
    assert(state.privilege() == PrivilegeLevel::Kernel);

    for (int i = 0; i < 9; ++i)
        assert(state.registers().read(static_cast<Register>(i)) == Word::zero());

    const Word pc = Word::from_integer(12345);
    const Word sp = Word::from_integer(987654);
    const Word ksp = Word::from_integer(456789);

    state.set_pc(pc);
    state.set_sp(sp);
    state.set_ksp(ksp);
    state.set_status(Comparison::Greater);
    state.set_privilege(PrivilegeLevel::User);

    state.registers().write(Register::R1, Word::from_integer(42));
    state.registers().write(Register::R7, Word::from_integer(-42));

    assert(state.pc() == pc);
    assert(state.sp() == sp);
    assert(state.ksp() == ksp);
    assert(state.status() == Comparison::Greater);
    assert(state.privilege() == PrivilegeLevel::User);
    assert(state.registers().read(Register::R1).to_integer() == 42);
    assert(state.registers().read(Register::R7).to_integer() == -42);

    state.set_status(Comparison::Less);
    assert(state.status() == Comparison::Less);

    state.set_status(Comparison::Equal);
    assert(state.status() == Comparison::Equal);

    state.set_privilege(PrivilegeLevel::Kernel);
    assert(state.privilege() == PrivilegeLevel::Kernel);

    state.set_ksp(Word::from_integer(111111));
    assert(state.ksp().to_integer() == 111111);

    static_assert(static_cast<std::int8_t>(TransitionCause::SystemCall) == 0);
    static_assert(static_cast<std::int8_t>(TransitionCause::Interrupt) == 1);
    static_assert(static_cast<std::int8_t>(TransitionCause::Exception) == 2);

    state.reset();

    assert(state.pc() == Word::zero());
    assert(state.sp() == Word::zero());
    assert(state.ksp() == Word::zero());
    assert(state.status() == Comparison::Equal);
    assert(state.privilege() == PrivilegeLevel::Kernel);

    for (int i = 0; i < 9; ++i)
        assert(state.registers().read(static_cast<Register>(i)) == Word::zero());

    return 0;
}

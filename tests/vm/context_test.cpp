#include <cassert>

#include "vm/context.hpp"

using ternary_machine::ternary::Word;
using ternary_machine::vm::Comparison;
using ternary_machine::vm::CPUState;
using ternary_machine::vm::PrivilegeLevel;
using ternary_machine::vm::Register;
using ternary_machine::vm::SavedContext;
using ternary_machine::vm::TransitionCause;

int main() {
    CPUState cpu;

    cpu.set_pc(Word::from_integer(1234));
    cpu.set_sp(Word::from_integer(5678));
    cpu.set_ksp(Word::from_integer(9999));
    cpu.set_status(Comparison::Greater);
    cpu.set_privilege(PrivilegeLevel::User);

    cpu.registers().write(Register::R0, Word::from_integer(10));
    cpu.registers().write(Register::R1, Word::from_integer(-20));
    cpu.registers().write(Register::R4, Word::from_integer(31415));
    cpu.registers().write(Register::R8, Word::from_integer(-2718));

    const SavedContext context = SavedContext::capture(cpu, TransitionCause::SystemCall);

    assert(context.pc == Word::from_integer(1234));
    assert(context.sp == Word::from_integer(5678));
    assert(context.status == Comparison::Greater);
    assert(context.privilege == PrivilegeLevel::User);
    assert(context.cause == TransitionCause::SystemCall);

    assert(context.registers.read(Register::R0).to_integer() == 10);
    assert(context.registers.read(Register::R1).to_integer() == -20);
    assert(context.registers.read(Register::R4).to_integer() == 31415);
    assert(context.registers.read(Register::R8).to_integer() == -2718);

    cpu.set_pc(Word::from_integer(777));
    cpu.set_sp(Word::from_integer(888));
    cpu.set_status(Comparison::Less);
    cpu.set_privilege(PrivilegeLevel::Kernel);

    cpu.registers().write(Register::R0, Word::zero());
    cpu.registers().write(Register::R1, Word::zero());
    cpu.registers().write(Register::R4, Word::zero());
    cpu.registers().write(Register::R8, Word::zero());

    context.restore(cpu);

    assert(cpu.pc() == Word::from_integer(1234));
    assert(cpu.sp() == Word::from_integer(5678));
    assert(cpu.status() == Comparison::Greater);
    assert(cpu.privilege() == PrivilegeLevel::User);

    assert(cpu.registers().read(Register::R0).to_integer() == 10);
    assert(cpu.registers().read(Register::R1).to_integer() == -20);
    assert(cpu.registers().read(Register::R4).to_integer() == 31415);
    assert(cpu.registers().read(Register::R8).to_integer() == -2718);

    cpu.set_pc(Word::from_integer(4321));
    cpu.set_sp(Word::from_integer(8765));
    cpu.set_status(Comparison::Less);
    cpu.set_privilege(PrivilegeLevel::Kernel);
    cpu.registers().write(Register::R3, Word::from_integer(999));

    const SavedContext interrupt_context = SavedContext::capture(cpu, TransitionCause::Interrupt);

    assert(interrupt_context.cause == TransitionCause::Interrupt);
    assert(interrupt_context.pc == Word::from_integer(4321));
    assert(interrupt_context.sp == Word::from_integer(8765));
    assert(interrupt_context.status == Comparison::Less);
    assert(interrupt_context.privilege == PrivilegeLevel::Kernel);
    assert(interrupt_context.registers.read(Register::R3).to_integer() == 999);

    const SavedContext exception_context = SavedContext::capture(cpu, TransitionCause::Exception);

    assert(exception_context.cause == TransitionCause::Exception);

    const SavedContext exception_context_copy = SavedContext::capture(cpu, TransitionCause::Exception);

    assert(exception_context.cause == exception_context_copy.cause);
    assert(exception_context.pc == exception_context_copy.pc);
    assert(exception_context.sp == exception_context_copy.sp);
    assert(exception_context.status == exception_context_copy.status);
    assert(exception_context.privilege == exception_context_copy.privilege);

    for (int i = 0; i < 9; ++i)
        assert(exception_context.registers.read(static_cast<Register>(i)) == exception_context_copy.registers.read(static_cast<Register>(i)));

    return 0;
}

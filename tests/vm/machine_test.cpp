#include <cassert>

#include "vm/machine.hpp"

using ternary_machine::ternary::Word;
using ternary_machine::vm::Comparison;
using ternary_machine::vm::ContextError;
using ternary_machine::vm::Machine;
using ternary_machine::vm::PrivilegeLevel;
using ternary_machine::vm::Register;
using ternary_machine::vm::TransitionCause;

int main() {
    Machine machine;

    assert(!machine.halted());
    assert(machine.cpu().pc() == Word::zero());
    assert(machine.cpu().sp() == Word::zero());
    assert(machine.cpu().status() == Comparison::Equal);
    assert(machine.memory().size() == 0);
    assert(machine.context_depth() == 0);

    {
        bool threw = false;

        try {
            machine.restore_context();
        } catch (const ContextError&) {
            threw = true;
        }

        assert(threw);
        assert(machine.context_depth() == 0);
    }

    machine.cpu().set_pc(Word::from_integer(100));
    machine.cpu().set_sp(Word::from_integer(500));
    machine.cpu().set_status(Comparison::Greater);
    machine.cpu().set_privilege(PrivilegeLevel::User);
    machine.cpu().registers().write(Register::R3, Word::from_integer(1234));

    const Word address = Word::from_integer(42);
    const Word value = Word::from_integer(-5678);

    machine.memory().write(address, value);
    machine.halt();

    assert(machine.halted());
    assert(machine.cpu().pc().to_integer() == 100);
    assert(machine.cpu().sp().to_integer() == 500);
    assert(machine.cpu().status() == Comparison::Greater);
    assert(machine.cpu().privilege() == PrivilegeLevel::User);
    assert(machine.cpu().registers().read(Register::R3).to_integer() == 1234);
    assert(machine.memory().read(address) == value);
    assert(machine.memory().size() == 1);

    machine.save_context(TransitionCause::SystemCall);

    assert(machine.context_depth() == 1);

    machine.cpu().set_pc(Word::from_integer(1000));
    machine.cpu().set_sp(Word::from_integer(2000));
    machine.cpu().set_status(Comparison::Less);
    machine.cpu().set_privilege(PrivilegeLevel::Kernel);
    machine.cpu().registers().write(Register::R3, Word::from_integer(9999));

    assert(machine.context_depth() == 1);

    machine.restore_context();

    assert(machine.context_depth() == 0);
    assert(machine.cpu().pc() == Word::from_integer(100));
    assert(machine.cpu().sp() == Word::from_integer(500));
    assert(machine.cpu().status() == Comparison::Greater);
    assert(machine.cpu().privilege() == PrivilegeLevel::User);
    assert(machine.cpu().registers().read(Register::R3).to_integer() == 1234);

    machine.cpu().set_pc(Word::from_integer(111));
    machine.cpu().set_sp(Word::from_integer(222));
    machine.cpu().set_status(Comparison::Less);
    machine.cpu().set_privilege(PrivilegeLevel::User);
    machine.cpu().registers().write(Register::R1, Word::from_integer(1111));

    machine.save_context(TransitionCause::SystemCall);

    machine.cpu().set_pc(Word::from_integer(333));
    machine.cpu().set_sp(Word::from_integer(444));
    machine.cpu().set_status(Comparison::Greater);
    machine.cpu().set_privilege(PrivilegeLevel::Kernel);
    machine.cpu().registers().write(Register::R1, Word::from_integer(2222));

    machine.save_context(TransitionCause::Interrupt);

    assert(machine.context_depth() == 2);

    machine.cpu().set_pc(Word::from_integer(555));
    machine.cpu().set_sp(Word::from_integer(666));
    machine.cpu().set_status(Comparison::Equal);
    machine.cpu().set_privilege(PrivilegeLevel::Kernel);
    machine.cpu().registers().write(Register::R1, Word::from_integer(3333));

    machine.restore_context();

    assert(machine.context_depth() == 1);
    assert(machine.cpu().pc() == Word::from_integer(333));
    assert(machine.cpu().sp() == Word::from_integer(444));
    assert(machine.cpu().status() == Comparison::Greater);
    assert(machine.cpu().privilege() == PrivilegeLevel::Kernel);
    assert(machine.cpu().registers().read(Register::R1).to_integer() == 2222);

    machine.restore_context();

    assert(machine.context_depth() == 0);
    assert(machine.cpu().pc() == Word::from_integer(111));
    assert(machine.cpu().sp() == Word::from_integer(222));
    assert(machine.cpu().status() == Comparison::Less);
    assert(machine.cpu().privilege() == PrivilegeLevel::User);
    assert(machine.cpu().registers().read(Register::R1).to_integer() == 1111);

    machine.halt();
    assert(machine.halted());

    machine.reset();

    assert(!machine.halted());
    assert(machine.context_depth() == 0);
    assert(machine.cpu().pc() == Word::zero());
    assert(machine.cpu().sp() == Word::zero());
    assert(machine.cpu().status() == Comparison::Equal);
    assert(machine.cpu().registers().read(Register::R1) == Word::zero());
    assert(machine.cpu().registers().read(Register::R3) == Word::zero());
    assert(machine.memory().size() == 0);
    assert(machine.memory().read(address) == Word::zero());

    return 0;
}

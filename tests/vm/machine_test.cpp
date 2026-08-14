#include <cassert>

#include "vm/machine.hpp"

using ternary_machine::ternary::Word;
using ternary_machine::vm::Comparison;
using ternary_machine::vm::Machine;
using ternary_machine::vm::Register;

int main() {
    Machine machine;

    assert(!machine.halted());
    assert(machine.cpu().pc() == Word::zero());
    assert(machine.cpu().sp() == Word::zero());
    assert(machine.cpu().status() == Comparison::Equal);
    assert(machine.memory().size() == 0);

    machine.cpu().set_pc(Word::from_integer(100));
    machine.cpu().set_sp(Word::from_integer(500));
    machine.cpu().set_status(Comparison::Greater);
    machine.cpu().registers().write(Register::R3, Word::from_integer(1234));

    const Word address = Word::from_integer(42);
    const Word value = Word::from_integer(-5678);

    machine.memory().write(address, value);
    machine.halt();

    assert(machine.halted());
    assert(machine.cpu().pc().to_integer() == 100);
    assert(machine.cpu().sp().to_integer() == 500);
    assert(machine.cpu().status() == Comparison::Greater);
    assert(machine.cpu().registers().read(Register::R3).to_integer() == 1234);
    assert(machine.memory().read(address) == value);
    assert(machine.memory().size() == 1);

    machine.reset();

    assert(!machine.halted());
    assert(machine.cpu().pc() == Word::zero());
    assert(machine.cpu().sp() == Word::zero());
    assert(machine.cpu().status() == Comparison::Equal);
    assert(machine.cpu().registers().read(Register::R3) == Word::zero());
    assert(machine.memory().size() == 0);
    assert(machine.memory().read(address) == Word::zero());

    return 0;
}

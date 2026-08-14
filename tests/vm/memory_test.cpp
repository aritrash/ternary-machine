#include <cassert>

#include "vm/memory.hpp"

using ternary_machine::ternary::Word;
using ternary_machine::vm::Memory;

int main() {
    Memory memory;

    assert(memory.size() == 0);

    const Word address_a = Word::from_integer(42);
    const Word address_b = Word::from_integer(1000000);

    const Word value_a = Word::from_integer(12345);
    const Word value_b = Word::from_integer(-54321);

    assert(memory.read(address_a) == Word::zero());
    assert(memory.read(address_b) == Word::zero());

    memory.write(address_a, value_a);

    assert(memory.read(address_a) == value_a);
    assert(memory.read(address_b) == Word::zero());
    assert(memory.size() == 1);

    memory.write(address_b, value_b);

    assert(memory.read(address_a) == value_a);
    assert(memory.read(address_b) == value_b);
    assert(memory.size() == 2);

    memory.write(address_a, value_b);

    assert(memory.read(address_a) == value_b);
    assert(memory.read(address_b) == value_b);
    assert(memory.size() == 2);

    memory.write(address_a, Word::zero());

    assert(memory.read(address_a) == Word::zero());
    assert(memory.read(address_b) == value_b);
    assert(memory.size() == 1);

    memory.clear();

    assert(memory.size() == 0);
    assert(memory.read(address_a) == Word::zero());
    assert(memory.read(address_b) == Word::zero());

    return 0;
}

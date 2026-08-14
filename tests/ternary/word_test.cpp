#include <cassert>
#include <cstdint>

#include "ternary/word.hpp"

using ternary_machine::ternary::Trit;
using ternary_machine::ternary::Word;

int main() {
    static_assert(Word::WIDTH == 27);
    static_assert(Word::TRYTES == 3);
    static_assert(Word::MIN_VALUE == -3812798742493LL);
    static_assert(Word::MAX_VALUE == 3812798742493LL);

    const Word zero = Word::zero();

    assert(zero.to_integer() == 0);
    assert(zero.to_string() == "000000000000000000000000000");

    for (std::size_t i = 0; i < Word::WIDTH; ++i)
        assert(zero.trit(i) == Trit::Zero);

    const Word one = Word::from_integer(1);
    const Word minus_one = Word::from_integer(-1);

    assert(one.to_integer() == 1);
    assert(one.mst() == Trit::Zero);
    assert(one.lst() == Trit::Pos);
    assert(one.to_string() == "000000000000000000000000001");

    assert(minus_one.to_integer() == -1);
    assert(minus_one.mst() == Trit::Zero);
    assert(minus_one.lst() == Trit::Neg);
    assert(minus_one.to_string() == "00000000000000000000000000n");

    const Word power = Word::from_integer(6561);

    assert(power.to_integer() == 6561);
    assert(power.mst() == Trit::Zero);
    assert(power.lst() == Trit::Zero);
    assert(power.trit(18) == Trit::Pos);

    for (std::size_t i = 0; i < Word::WIDTH; ++i) {
        if (i != 18)
            assert(power.trit(i) == Trit::Zero);
    }

    const Word maximum = Word::from_integer(Word::MAX_VALUE);
    const Word minimum = Word::from_integer(Word::MIN_VALUE);

    assert(maximum.to_integer() == Word::MAX_VALUE);
    assert(minimum.to_integer() == Word::MIN_VALUE);

    for (std::size_t i = 0; i < Word::WIDTH; ++i) {
        assert(maximum.trit(i) == Trit::Pos);
        assert(minimum.trit(i) == Trit::Neg);
    }

    assert(maximum.to_string() == "111111111111111111111111111");
    assert(minimum.to_string() == "nnnnnnnnnnnnnnnnnnnnnnnnnnn");

    const Word pattern = Word::from_integer(9841);

    assert(pattern.to_integer() == 9841);

    for (std::int64_t value = -1000000; value <= 1000000; ++value) {
        const Word word = Word::from_integer(value);
        assert(word.to_integer() == value);
    }

    return 0;
}

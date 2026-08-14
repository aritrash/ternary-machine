#include <cassert>
#include <cstdint>

#include "vm/alu.hpp"

using ternary_machine::ternary::Word;
using ternary_machine::vm::ALU;
using ternary_machine::vm::Comparison;
using ternary_machine::ternary::Trit;

int main() {
    const Word zero = Word::zero();
    const Word one = Word::from_integer(1);
    const Word two = Word::from_integer(2);
    const Word five = Word::from_integer(5);
    const Word seven = Word::from_integer(7);
    const Word negative_five = Word::from_integer(-5);
    const Word negative_seven = Word::from_integer(-7);

    assert(ALU::add(zero, zero).to_integer() == 0);
    assert(ALU::add(five, seven).to_integer() == 12);
    assert(ALU::add(negative_five, seven).to_integer() == 2);
    assert(ALU::add(negative_five, negative_seven).to_integer() == -12);

    assert(ALU::sub(zero, zero).to_integer() == 0);
    assert(ALU::sub(seven, five).to_integer() == 2);
    assert(ALU::sub(five, seven).to_integer() == -2);
    assert(ALU::sub(negative_five, seven).to_integer() == -12);
    assert(ALU::sub(negative_five, negative_seven).to_integer() == 2);

    assert(ALU::mul(zero, seven).to_integer() == 0);
    assert(ALU::mul(one, seven).to_integer() == 7);
    assert(ALU::mul(five, seven).to_integer() == 35);
    assert(ALU::mul(negative_five, seven).to_integer() == -35);
    assert(ALU::mul(negative_five, negative_seven).to_integer() == 35);
    
    {
        const Word neg = Word::from_integer(-1);
        const Word zero = Word::zero();
        const Word pos = Word::from_integer(1);

        assert(ALU::tand(neg, neg).to_integer() == -1);
        assert(ALU::tand(neg, zero).to_integer() == -1);
        assert(ALU::tand(neg, pos).to_integer() == -1);
        assert(ALU::tand(zero, neg).to_integer() == -1);
        assert(ALU::tand(zero, zero).to_integer() == 0);
        assert(ALU::tand(zero, pos).to_integer() == 0);
        assert(ALU::tand(pos, neg).to_integer() == -1);
        assert(ALU::tand(pos, zero).to_integer() == 0);
        assert(ALU::tand(pos, pos).to_integer() == 1);
    }

    {
        const Word neg = Word::from_integer(-1);
        const Word zero = Word::zero();
        const Word pos = Word::from_integer(1);

        assert(ALU::tor(neg, neg).to_integer() == -1);
        assert(ALU::tor(neg, zero).to_integer() == 0);
        assert(ALU::tor(neg, pos).to_integer() == 1);
        assert(ALU::tor(zero, neg).to_integer() == 0);
        assert(ALU::tor(zero, zero).to_integer() == 0);
        assert(ALU::tor(zero, pos).to_integer() == 1);
        assert(ALU::tor(pos, neg).to_integer() == 1);
        assert(ALU::tor(pos, zero).to_integer() == 1);
        assert(ALU::tor(pos, pos).to_integer() == 1);
    }

    {
        const Word neg = Word::from_integer(-1);
        const Word zero = Word::zero();
        const Word pos = Word::from_integer(1);

        assert(ALU::txor(neg, neg).to_integer() == 0);
        assert(ALU::txor(neg, zero).to_integer() == -1);
        assert(ALU::txor(neg, pos).to_integer() == 1);
        assert(ALU::txor(zero, neg).to_integer() == -1);
        assert(ALU::txor(zero, zero).to_integer() == 0);
        assert(ALU::txor(zero, pos).to_integer() == -1);
        assert(ALU::txor(pos, neg).to_integer() == 1);
        assert(ALU::txor(pos, zero).to_integer() == -1);
        assert(ALU::txor(pos, pos).to_integer() == 0);
    }

    {
        assert(ALU::tnot(Word::from_integer(-1)).to_integer() == 1);
        assert(ALU::tnot(Word::zero()).to_integer() == 0);
        assert(ALU::tnot(Word::from_integer(1)).to_integer() == -1);
    }

    {
        const Word lhs = Word::from_integer(-6561);
        const Word rhs = Word::from_integer(2187);

        const Word tand = ALU::tand(lhs, rhs);
        const Word tor = ALU::tor(lhs, rhs);
        const Word txor = ALU::txor(lhs, rhs);
        const Word tnot = ALU::tnot(lhs);

        for (std::size_t i = 0; i < Word::WIDTH; ++i) {
            const Trit a = lhs.trit(i);
            const Trit b = rhs.trit(i);

            assert(tand.trit(i) == (static_cast<int>(a) < static_cast<int>(b) ? a : b));
            assert(tor.trit(i) == (static_cast<int>(a) > static_cast<int>(b) ? a : b));
        }

        assert(tnot.to_integer() == 6561);
        assert(txor.to_integer() != lhs.to_integer());
    }
    
    {
        const Word value = Word::from_integer(1);

        assert(ALU::shift(value, 0) == value);
        assert(ALU::shift(value, 1).to_integer() == 3);
        assert(ALU::shift(value, 2).to_integer() == 9);
        assert(ALU::shift(value, -1).to_integer() == 0);
    }

    {
        Word value = Word::zero();

        for (std::size_t i = 0; i < Word::WIDTH; ++i)
            value.set_trit(i, i % 3 == 0 ? Trit::Neg : i % 3 == 1 ? Trit::Zero : Trit::Pos);

        const Word left = ALU::shift(value, 3);
        const Word right = ALU::shift(value, -3);

        for (std::size_t i = 0; i < Word::WIDTH; ++i) {
            if (i < Word::WIDTH - 3)
                assert(left.trit(i) == value.trit(i + 3));
            else
                assert(left.trit(i) == Trit::Zero);

            if (i < 3)
                assert(right.trit(i) == Trit::Zero);
            else
                assert(right.trit(i) == value.trit(i - 3));
        }
    }

    {
        const Word value = Word::from_integer(12345);

        assert(ALU::shift(value, 27) == Word::zero());
        assert(ALU::shift(value, -27) == Word::zero());
        assert(ALU::shift(value, 100) == Word::zero());
        assert(ALU::shift(value, -100) == Word::zero());
    }

    assert(ALU::compare(five, seven) == Comparison::Less);
    assert(ALU::compare(seven, five) == Comparison::Greater);
    assert(ALU::compare(seven, seven) == Comparison::Equal);
    assert(ALU::compare(negative_five, five) == Comparison::Less);
    assert(ALU::compare(five, negative_five) == Comparison::Greater);
    assert(ALU::compare(negative_five, negative_five) == Comparison::Equal);

    const Word max_word = Word::from_integer(ALU::HALF_MODULUS);
    const Word min_word = Word::from_integer(-ALU::HALF_MODULUS);

    assert(ALU::add(max_word, one).to_integer() == -ALU::HALF_MODULUS);
    assert(ALU::sub(min_word, one).to_integer() == ALU::HALF_MODULUS);

    assert(ALU::add(max_word, max_word).to_integer() == -1);
    assert(ALU::sub(min_word, min_word).to_integer() == 0);

    const Word large_a = Word::from_integer(1000000);
    const Word large_b = Word::from_integer(1000000);

    assert(ALU::mul(large_a, large_b).to_integer() == 1000000000000LL);

    return 0;
}

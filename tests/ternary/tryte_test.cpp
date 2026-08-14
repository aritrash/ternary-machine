#include <cassert>
#include <cstdint>
#include <string>

#include "ternary/tryte.hpp"

using ternary_machine::ternary::Trit;
using ternary_machine::ternary::Tryte;

int main() {
    static_assert(Tryte::WIDTH == 9);
    static_assert(Tryte::MIN_VALUE == -9841);
    static_assert(Tryte::MAX_VALUE == 9841);

    const Tryte zero = Tryte::zero();

    assert(zero.to_integer() == 0);
    assert(zero.to_string() == "000000000");

    assert(Tryte::from_integer(1).to_string() == "000000001");
    assert(Tryte::from_integer(-1).to_string() == "00000000n");
    assert(Tryte::from_integer(6561).to_string() == "100000000");
    assert(Tryte::from_integer(-6561).to_string() == "n00000000");

    const Tryte maximum = Tryte::from_integer(Tryte::MAX_VALUE);
    const Tryte minimum = Tryte::from_integer(Tryte::MIN_VALUE);

    assert(maximum.to_string() == "111111111");
    assert(minimum.to_string() == "nnnnnnnnn");
    assert(maximum.to_integer() == Tryte::MAX_VALUE);
    assert(minimum.to_integer() == Tryte::MIN_VALUE);

    assert(zero.mst() == Trit::Zero);
    assert(zero.lst() == Trit::Zero);
    assert(maximum.mst() == Trit::Pos);
    assert(maximum.lst() == Trit::Pos);
    assert(minimum.mst() == Trit::Neg);
    assert(minimum.lst() == Trit::Neg);

    for (std::int32_t value = Tryte::MIN_VALUE; value <= Tryte::MAX_VALUE; ++value) {
        const Tryte tryte = Tryte::from_integer(value);
        assert(tryte.to_integer() == value);
    }

    return 0;
}

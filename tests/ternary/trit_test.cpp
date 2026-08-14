#include <cassert>
#include "ternary/trit.hpp"

using tm::ternary::Trit;

int main() {
    static_assert(sizeof(Trit) == sizeof(std::int8_t));

    assert(static_cast<std::int8_t>(Trit::Neg) == -1);
    assert(static_cast<std::int8_t>(Trit::Zero) == 0);
    assert(static_cast<std::int8_t>(Trit::Pos) == 1);

    assert(tm::ternary::is_valid(Trit::Neg));
    assert(tm::ternary::is_valid(Trit::Zero));
    assert(tm::ternary::is_valid(Trit::Pos));

    assert(tm::ternary::negate(Trit::Neg) == Trit::Pos);
    assert(tm::ternary::negate(Trit::Zero) == Trit::Zero);
    assert(tm::ternary::negate(Trit::Pos) == Trit::Neg);

    return 0;
}

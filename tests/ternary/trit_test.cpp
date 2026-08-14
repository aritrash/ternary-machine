#include <cassert>
#include "ternary/trit.hpp"

using ternary_machine::ternary::Trit;

int main() {
    static_assert(sizeof(Trit) == sizeof(std::int8_t));

    assert(static_cast<std::int8_t>(Trit::Neg) == -1);
    assert(static_cast<std::int8_t>(Trit::Zero) == 0);
    assert(static_cast<std::int8_t>(Trit::Pos) == 1);

    assert(ternary_machine::ternary::is_valid(Trit::Neg));
    assert(ternary_machine::ternary::is_valid(Trit::Zero));
    assert(ternary_machine::ternary::is_valid(Trit::Pos));

    assert(ternary_machine::ternary::negate(Trit::Neg) == Trit::Pos);
    assert(ternary_machine::ternary::negate(Trit::Zero) == Trit::Zero);
    assert(ternary_machine::ternary::negate(Trit::Pos) == Trit::Neg);

    return 0;
}

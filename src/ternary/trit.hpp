#pragma once

#include <cstdint>

namespace ternary_machine::ternary {

	enum class Trit : std::int8_t {
	    Neg = -1,
	    Zero = 0,
	    Pos = 1
	};

	constexpr bool is_valid(Trit value) noexcept {
	    return value == Trit::Neg || value == Trit::Zero || value == Trit::Pos;
	}

	constexpr Trit negate(Trit value) noexcept {
	    switch (value) {
		case Trit::Neg: return Trit::Pos;
		case Trit::Zero: return Trit::Zero;
		case Trit::Pos: return Trit::Neg;
	    }
	    return Trit::Zero;
	}

}

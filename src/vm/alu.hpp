#pragma once

#include <cstddef>
#include <cstdint>

#include "cpu_state.hpp"
#include "ternary/word.hpp"

namespace ternary_machine::vm {

class ALU final {
public:
    static constexpr std::int64_t MODULUS = 7625597484987LL;
    static constexpr std::int64_t HALF_MODULUS = 3812798742493LL;

    [[nodiscard]] static ternary::Word add(const ternary::Word& lhs, const ternary::Word& rhs) noexcept {
        return ternary::Word::from_integer(wrap(static_cast<__int128>(lhs.to_integer()) + rhs.to_integer()));
    }

    [[nodiscard]] static ternary::Word sub(const ternary::Word& lhs, const ternary::Word& rhs) noexcept {
        return ternary::Word::from_integer(wrap(static_cast<__int128>(lhs.to_integer()) - rhs.to_integer()));
    }

    [[nodiscard]] static ternary::Word mul(const ternary::Word& lhs, const ternary::Word& rhs) noexcept {
        return ternary::Word::from_integer(wrap(static_cast<__int128>(lhs.to_integer()) * rhs.to_integer()));
    }
    
        [[nodiscard]] static ternary::Word tand(const ternary::Word& lhs, const ternary::Word& rhs) noexcept {
        ternary::Word result;

        for (std::size_t i = 0; i < ternary::Word::WIDTH; ++i) {
            const auto a = lhs.trit(i);
            const auto b = rhs.trit(i);
            result.set_trit(i, static_cast<int>(a) < static_cast<int>(b) ? a : b);
        }

        return result;
    }

    [[nodiscard]] static ternary::Word tor(const ternary::Word& lhs, const ternary::Word& rhs) noexcept {
        ternary::Word result;

        for (std::size_t i = 0; i < ternary::Word::WIDTH; ++i) {
            const auto a = lhs.trit(i);
            const auto b = rhs.trit(i);
            result.set_trit(i, static_cast<int>(a) > static_cast<int>(b) ? a : b);
        }

        return result;
    }

    [[nodiscard]] static ternary::Word txor(const ternary::Word& lhs, const ternary::Word& rhs) noexcept {
        ternary::Word result;

        for (std::size_t i = 0; i < ternary::Word::WIDTH; ++i) {
            const int a = static_cast<int>(lhs.trit(i));
            const int b = static_cast<int>(rhs.trit(i));
            int value;

            if (a == b)
                value = 0;
            else if ((a == -1 && b == 0) || (a == 0 && b == -1))
                value = -1;
            else if ((a == 0 && b == 1) || (a == 1 && b == 0))
                value = -1;
            else
                value = 1;

            result.set_trit(i, static_cast<ternary::Trit>(value));
        }

        return result;
    }

    [[nodiscard]] static ternary::Word tnot(const ternary::Word& value) noexcept {
        ternary::Word result;

        for (std::size_t i = 0; i < ternary::Word::WIDTH; ++i)
            result.set_trit(i, static_cast<ternary::Trit>(-static_cast<int>(value.trit(i))));

        return result;
    }
    
    [[nodiscard]] static ternary::Word shift(const ternary::Word& value, std::int64_t amount) noexcept {
		ternary::Word result = ternary::Word::zero();

		if (amount >= static_cast<std::int64_t>(ternary::Word::WIDTH) || amount <= -static_cast<std::int64_t>(ternary::Word::WIDTH))
		    return result;

		if (amount > 0) {
		    const auto shift = static_cast<std::size_t>(amount);

		    for (std::size_t i = shift; i < ternary::Word::WIDTH; ++i)
		        result.set_trit(i - shift, value.trit(i));
		} else if (amount < 0) {
		    const auto shift = static_cast<std::size_t>(-amount);

		    for (std::size_t i = 0; i + shift < ternary::Word::WIDTH; ++i)
		        result.set_trit(i + shift, value.trit(i));
		} else {
		    result = value;
		}

		return result;
	}

    [[nodiscard]] static Comparison compare(const ternary::Word& lhs, const ternary::Word& rhs) noexcept {
        const auto left = lhs.to_integer();
        const auto right = rhs.to_integer();

        if (left < right)
            return Comparison::Less;

        if (left > right)
            return Comparison::Greater;

        return Comparison::Equal;
    }

private:
    [[nodiscard]] static std::int64_t wrap(__int128 value) noexcept {
        value %= MODULUS;

        if (value > HALF_MODULUS)
            value -= MODULUS;
        else if (value < -HALF_MODULUS)
            value += MODULUS;

        return static_cast<std::int64_t>(value);
    }
};

}

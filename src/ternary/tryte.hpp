#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "trit.hpp"

namespace ternary_machine::ternary {

class Tryte final {
	public:
	    static constexpr std::size_t WIDTH = 9;
	    static constexpr std::int32_t MIN_VALUE = -9841;
	    static constexpr std::int32_t MAX_VALUE = 9841;

	    constexpr Tryte() noexcept = default;

	    static constexpr Tryte zero() noexcept {
		return Tryte{};
	    }

	    static Tryte from_integer(std::int32_t value) {
		if (value < MIN_VALUE || value > MAX_VALUE) throw std::out_of_range("value does not fit in a Tryte");

		Tryte result;
		std::int32_t remaining = value;

		for (std::size_t i = WIDTH; i-- > 0;) {
		    const std::int32_t place = power_of_three(i);
		    std::int32_t digit = remaining / place;
		    std::int32_t remainder = remaining % place;

		    if (remainder > place / 2) ++digit;
		    else if (remainder < -(place / 2)) --digit;

		    if (digit > 1) digit = 1;
		    if (digit < -1) digit = -1;

		    result.trits_[WIDTH - 1 - i] = static_cast<Trit>(digit);
		    remaining -= digit * place;
		}

		return result;
	    }

	    constexpr Trit operator[](std::size_t index) const noexcept {
		return trits_[index];
	    }

	    constexpr void set(std::size_t index, Trit value) noexcept {
		trits_[index] = value;
	    }

	    constexpr Trit mst() const noexcept {
		return trits_[0];
	    }

	    constexpr Trit lst() const noexcept {
		return trits_[WIDTH - 1];
	    }

	    [[nodiscard]] std::int32_t to_integer() const noexcept {
		std::int32_t value = 0;

		for (std::size_t i = 0; i < WIDTH; ++i)
		    value += static_cast<std::int8_t>(trits_[i]) * power_of_three(WIDTH - 1 - i);

		return value;
	    }

	    [[nodiscard]] std::string to_string() const {
		std::string result;
		result.reserve(WIDTH);

		for (const Trit trit : trits_) {
		    switch (trit) {
		        case Trit::Neg: result += 'n'; break;
		        case Trit::Zero: result += '0'; break;
		        case Trit::Pos: result += '1'; break;
		    }
		}

		return result;
	    }

	    constexpr bool operator==(const Tryte&) const noexcept = default;

	private:
	    static constexpr std::int32_t power_of_three(std::size_t exponent) noexcept {
		std::int32_t result = 1;
		for (std::size_t i = 0; i < exponent; ++i) result *= 3;
		return result;
	    }

	    std::array<Trit, WIDTH> trits_{};
	};

}

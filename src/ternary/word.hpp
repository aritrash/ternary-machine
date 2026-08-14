#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "tryte.hpp"

namespace ternary_machine::ternary {

class Word final {
	public:
	    static constexpr std::size_t WIDTH = 27;
	    static constexpr std::size_t TRYTES = 3;
	    static constexpr std::int64_t MIN_VALUE = -3812798742493LL;
	    static constexpr std::int64_t MAX_VALUE = 3812798742493LL;

	    constexpr Word() noexcept = default;

	    static constexpr Word zero() noexcept {
		return Word{};
	    }

	    static Word from_integer(std::int64_t value) {
		if (value < MIN_VALUE || value > MAX_VALUE) throw std::out_of_range("value does not fit in a Word");

		Word result;
		std::int64_t remaining = value;

		for (std::size_t i = WIDTH; i-- > 0;) {
		    std::int64_t remainder = remaining % 3;

		    if (remainder == 2) {
		        result.set_trit(i, Trit::Neg);
		        remaining = (remaining + 1) / 3;
		    } else if (remainder == -2) {
		        result.set_trit(i, Trit::Pos);
		        remaining = (remaining - 1) / 3;
		    } else {
		        result.set_trit(i, static_cast<Trit>(remainder));
		        remaining /= 3;
		    }
		}

		if (remaining != 0) throw std::out_of_range("value does not fit in a Word");
		return result;
	    }

	    constexpr Tryte operator[](std::size_t index) const noexcept {
		return trytes_[index];
	    }

	    constexpr void set_tryte(std::size_t index, Tryte value) noexcept {
		trytes_[index] = value;
	    }

	    constexpr Trit trit(std::size_t index) const noexcept {
		const std::size_t tryte_index = index / Tryte::WIDTH;
		const std::size_t trit_index = index % Tryte::WIDTH;
		return trytes_[tryte_index][trit_index];
	    }

	    constexpr void set_trit(std::size_t index, Trit value) noexcept {
		const std::size_t tryte_index = index / Tryte::WIDTH;
		const std::size_t trit_index = index % Tryte::WIDTH;
		trytes_[tryte_index].set(trit_index, value);
	    }

	    constexpr Trit mst() const noexcept {
		return trit(0);
	    }

	    constexpr Trit lst() const noexcept {
		return trit(WIDTH - 1);
	    }

	    [[nodiscard]] std::int64_t to_integer() const noexcept {
		std::int64_t value = 0;

		for (std::size_t i = 0; i < WIDTH; ++i)
		    value = value * 3 + static_cast<std::int8_t>(trit(i));

		return value;
	    }

	    [[nodiscard]] std::string to_string() const {
		std::string result;
		result.reserve(WIDTH);

		for (std::size_t i = 0; i < WIDTH; ++i) {
		    switch (trit(i)) {
		        case Trit::Neg: result += 'n'; break;
		        case Trit::Zero: result += '0'; break;
		        case Trit::Pos: result += '1'; break;
		    }
		}

		return result;
	    }

	    constexpr bool operator==(const Word&) const noexcept = default;

	private:
	    std::array<Tryte, TRYTES> trytes_{};
	};
}

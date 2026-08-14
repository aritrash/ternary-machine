#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "ternary/word.hpp"

namespace ternary_machine::vm {

enum class Register : std::uint8_t {
    R0 = 0,
    R1 = 1,
    R2 = 2,
    R3 = 3,
    R4 = 4,
    R5 = 5,
    R6 = 6,
    R7 = 7,
    R8 = 8
};

class RegisterFile final {
public:
    static constexpr std::size_t COUNT = 9;

    constexpr RegisterFile() noexcept = default;

    [[nodiscard]] constexpr ternary::Word read(Register reg) const noexcept {
        return registers_[index(reg)];
    }

    constexpr void write(Register reg, ternary::Word value) noexcept {
        registers_[index(reg)] = value;
    }

    constexpr void clear() noexcept {
        registers_.fill(ternary::Word::zero());
    }

private:
    static constexpr std::size_t index(Register reg) noexcept {
        return static_cast<std::size_t>(reg);
    }

    std::array<ternary::Word, COUNT> registers_{};
};

}

#pragma once

#include "register_file.hpp"

namespace ternary_machine::vm {

enum class Comparison : std::int8_t {
    Less = -1,
    Equal = 0,
    Greater = 1
};

class CPUState final {
public:
    constexpr CPUState() noexcept = default;

    [[nodiscard]] constexpr const RegisterFile& registers() const noexcept {
        return registers_;
    }

    [[nodiscard]] constexpr RegisterFile& registers() noexcept {
        return registers_;
    }

    [[nodiscard]] constexpr ternary::Word pc() const noexcept {
        return pc_;
    }

    constexpr void set_pc(ternary::Word value) noexcept {
        pc_ = value;
    }

    [[nodiscard]] constexpr ternary::Word sp() const noexcept {
        return sp_;
    }

    constexpr void set_sp(ternary::Word value) noexcept {
        sp_ = value;
    }

    [[nodiscard]] constexpr Comparison status() const noexcept {
        return status_;
    }

    constexpr void set_status(Comparison value) noexcept {
        status_ = value;
    }

    constexpr void reset() noexcept {
        registers_.clear();
        pc_ = ternary::Word::zero();
        sp_ = ternary::Word::zero();
        status_ = Comparison::Equal;
    }

private:
    RegisterFile registers_{};
    ternary::Word pc_{};
    ternary::Word sp_{};
    Comparison status_ = Comparison::Equal;
};

}

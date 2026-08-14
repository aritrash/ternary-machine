#pragma once

#include "cpu_state.hpp"
#include "memory.hpp"

namespace ternary_machine::vm {

class Machine final {
public:
    constexpr Machine() noexcept = default;

    [[nodiscard]] constexpr CPUState& cpu() noexcept {
        return cpu_;
    }

    [[nodiscard]] constexpr const CPUState& cpu() const noexcept {
        return cpu_;
    }

    [[nodiscard]] Memory& memory() noexcept {
        return memory_;
    }

    [[nodiscard]] const Memory& memory() const noexcept {
        return memory_;
    }

    [[nodiscard]] constexpr bool halted() const noexcept {
        return halted_;
    }

    constexpr void halt() noexcept {
        halted_ = true;
    }

    void reset() noexcept {
        cpu_.reset();
        memory_.clear();
        halted_ = false;
    }

private:
    CPUState cpu_{};
    Memory memory_{};
    bool halted_ = false;
};

}

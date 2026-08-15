#pragma once

#include <stdexcept>
#include <vector>

#include "cpu_state.hpp"
#include "memory.hpp"
#include "context.hpp"

namespace ternary_machine::vm {

class ContextError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

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

    [[nodiscard]] std::size_t context_depth() const noexcept {
        return contexts_.size();
    }

    void save_context(TransitionCause cause) {
        contexts_.push_back(SavedContext::capture(cpu_, cause));
    }

    void restore_context() {
        if (contexts_.empty())
            throw ContextError("context stack is empty");

        contexts_.back().restore(cpu_);
        contexts_.pop_back();
    }

    void reset() noexcept {
        cpu_.reset();
        memory_.clear();
        contexts_.clear();
        halted_ = false;
    }

private:
    CPUState cpu_{};
    Memory memory_{};
    std::vector<SavedContext> contexts_{};
    bool halted_ = false;
};

}

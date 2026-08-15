#pragma once

#include <cstddef>
#include <cstdint>

#include "ternary/word.hpp"

namespace ternary_machine::vm {

class VectorTable final {
public:
    using Address = ternary::Word;
    using Cause = std::uint8_t;

    static constexpr std::size_t ResetAddress = 0;
    static constexpr std::size_t VectorBase = 1;
    static constexpr std::size_t SystemCallVector = VectorBase;
    static constexpr std::size_t InterruptVectorBase = VectorBase + 1;
    static constexpr std::size_t ExceptionVectorBase = VectorBase + 28;
    static constexpr Cause MaxCause = 26;
    static constexpr std::size_t InterruptVectorCount = MaxCause + 1;
    static constexpr std::size_t ExceptionVectorCount = MaxCause + 1;
    static constexpr std::size_t VectorTableWords = 1 + InterruptVectorCount + ExceptionVectorCount;

    [[nodiscard]] static constexpr bool valid_cause(Cause cause) noexcept {
        return cause <= MaxCause;
    }

    [[nodiscard]] static Address system_call_vector() noexcept {
        return Address::from_integer(static_cast<std::int64_t>(SystemCallVector));
    }

    [[nodiscard]] static Address interrupt_vector(Cause cause) noexcept {
        return Address::from_integer(static_cast<std::int64_t>(InterruptVectorBase + cause));
    }

    [[nodiscard]] static Address exception_vector(Cause cause) noexcept {
        return Address::from_integer(static_cast<std::int64_t>(ExceptionVectorBase + cause));
    }

    [[nodiscard]] static constexpr std::size_t interrupt_vector_address(Cause cause) noexcept {
        return InterruptVectorBase + cause;
    }

    [[nodiscard]] static constexpr std::size_t exception_vector_address(Cause cause) noexcept {
        return ExceptionVectorBase + cause;
    }

private:
    VectorTable() = delete;
};

}

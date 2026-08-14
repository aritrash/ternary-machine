#pragma once

#include <cstdint>
#include <unordered_map>

#include "ternary/word.hpp"

namespace ternary_machine::vm {

class Memory final {
public:
    using Address = ternary::Word;
    using Value = ternary::Word;

    Memory() = default;

    [[nodiscard]] Value read(Address address) const {
        const auto key = address.to_integer();
        const auto it = storage_.find(key);
        return it == storage_.end() ? Value::zero() : it->second;
    }

    void write(Address address, Value value) {
        const auto key = address.to_integer();

        if (value == Value::zero()) {
            storage_.erase(key);
            return;
        }

        storage_[key] = value;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return storage_.size();
    }

    void clear() noexcept {
        storage_.clear();
    }

private:
    std::unordered_map<std::int64_t, Value> storage_;
};

}

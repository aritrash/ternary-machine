#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ternary/word.hpp"

namespace ternary_machine::runtime {

class LoadedImage final {
public:
    enum class SectionType : std::int8_t {
        Text,
        Data,
        Rodata,
        Bss
    };

    enum class SectionFlags : std::int8_t {
        None = 0,
        Read = 1,
        Write = 2,
        Execute = 4
    };

    struct Section final {
        SectionType type = SectionType::Text;
        std::int8_t flags = static_cast<std::int8_t>(SectionFlags::None);
        std::int64_t virtual_address = 0;
        std::int64_t memory_size = 0;
        std::int64_t alignment = 1;
        std::vector<ternary::Word> words;

        [[nodiscard]] constexpr bool readable() const noexcept {
            return (flags & static_cast<std::int8_t>(SectionFlags::Read)) != 0;
        }

        [[nodiscard]] constexpr bool writable() const noexcept {
            return (flags & static_cast<std::int8_t>(SectionFlags::Write)) != 0;
        }

        [[nodiscard]] constexpr bool executable() const noexcept {
            return (flags & static_cast<std::int8_t>(SectionFlags::Execute)) != 0;
        }

        [[nodiscard]] constexpr bool has_payload() const noexcept {
            return !words.empty();
        }

        bool operator==(const Section&) const = default;
    };

    struct Symbol final {
        std::int64_t identifier = 0;
        std::int8_t type = 0;
        std::int64_t section = 0;
        std::int64_t address = 0;

        bool operator==(const Symbol&) const = default;
    };

    LoadedImage() = default;

    [[nodiscard]] constexpr std::int64_t architecture_id() const noexcept {
        return architecture_id_;
    }

    constexpr void set_architecture_id(std::int64_t value) noexcept {
        architecture_id_ = value;
    }

    [[nodiscard]] constexpr std::int64_t isa_version() const noexcept {
        return isa_version_;
    }

    constexpr void set_isa_version(std::int64_t value) noexcept {
        isa_version_ = value;
    }

    [[nodiscard]] constexpr std::int64_t format_version() const noexcept {
        return format_version_;
    }

    constexpr void set_format_version(std::int64_t value) noexcept {
        format_version_ = value;
    }

    [[nodiscard]] constexpr std::int64_t flags() const noexcept {
        return flags_;
    }

    constexpr void set_flags(std::int64_t value) noexcept {
        flags_ = value;
    }

    [[nodiscard]] constexpr std::int64_t entry_point() const noexcept {
        return entry_point_;
    }

    constexpr void set_entry_point(std::int64_t value) noexcept {
        entry_point_ = value;
    }

    [[nodiscard]] constexpr std::int64_t start_symbol() const noexcept {
        return start_symbol_;
    }

    constexpr void set_start_symbol(std::int64_t value) noexcept {
        start_symbol_ = value;
    }

    [[nodiscard]] constexpr std::int64_t memory_base() const noexcept {
        return memory_base_;
    }

    constexpr void set_memory_base(std::int64_t value) noexcept {
        memory_base_ = value;
    }

    [[nodiscard]] constexpr std::int64_t memory_size() const noexcept {
        return memory_size_;
    }

    constexpr void set_memory_size(std::int64_t value) noexcept {
        memory_size_ = value;
    }

    [[nodiscard]] const std::vector<Section>& sections() const noexcept {
        return sections_;
    }

    [[nodiscard]] std::vector<Section>& sections() noexcept {
        return sections_;
    }

    [[nodiscard]] const std::vector<Symbol>& symbols() const noexcept {
        return symbols_;
    }

    [[nodiscard]] std::vector<Symbol>& symbols() noexcept {
        return symbols_;
    }

    [[nodiscard]] const Section* section(SectionType type) const noexcept {
        for (const auto& candidate : sections_)
            if (candidate.type == type)
                return &candidate;

        return nullptr;
    }

    [[nodiscard]] Section* section(SectionType type) noexcept {
        for (auto& candidate : sections_)
            if (candidate.type == type)
                return &candidate;

        return nullptr;
    }

    [[nodiscard]] const Section* text() const noexcept {
        return section(SectionType::Text);
    }

    [[nodiscard]] const Section* data() const noexcept {
        return section(SectionType::Data);
    }

    [[nodiscard]] const Section* rodata() const noexcept {
        return section(SectionType::Rodata);
    }

    [[nodiscard]] const Section* bss() const noexcept {
        return section(SectionType::Bss);
    }

    [[nodiscard]] bool has_section(SectionType type) const noexcept {
        return section(type) != nullptr;
    }

    [[nodiscard]] bool is_valid_entry_point() const noexcept {
        const auto* text_section = text();

        if (text_section == nullptr)
            return false;

        const auto begin = text_section->virtual_address;
        const auto end = begin + text_section->memory_size;

        return entry_point_ >= begin && entry_point_ < end;
    }

    bool operator==(const LoadedImage&) const = default;

private:
    std::int64_t architecture_id_ = 0;
    std::int64_t isa_version_ = 0;
    std::int64_t format_version_ = 0;
    std::int64_t flags_ = 0;
    std::int64_t entry_point_ = 0;
    std::int64_t start_symbol_ = -1;
    std::int64_t memory_base_ = 0;
    std::int64_t memory_size_ = 0;

    std::vector<Section> sections_;
    std::vector<Symbol> symbols_;
};

}

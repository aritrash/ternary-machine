#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "assembler/ir.hpp"
#include "assembler/symbol_table.hpp"
#include "ternary/word.hpp"

namespace ternary_machine::assembler {

enum class SectionType : std::uint8_t {
    Text,
    Data,
    Rodata,
    Bss
};

enum class SectionFlags : std::uint8_t {
    None = 0,
    Executable = 1 << 0,
    Writable = 1 << 1,
    ReadOnly = 1 << 2
};

constexpr SectionFlags operator|(SectionFlags lhs, SectionFlags rhs) noexcept {
    return static_cast<SectionFlags>(
        static_cast<std::uint8_t>(lhs) |
        static_cast<std::uint8_t>(rhs)
    );
}

constexpr bool has_flag(SectionFlags value, SectionFlags flag) noexcept {
    return (
        static_cast<std::uint8_t>(value) &
        static_cast<std::uint8_t>(flag)
    ) != 0;
}

struct SectionLayout final {
    std::string name;
    SectionType type = SectionType::Text;
    SectionFlags flags = SectionFlags::None;
    std::size_t file_offset = 0;
    std::size_t file_size = 0;
    ternary::Word virtual_address = ternary::Word::zero();
    std::size_t memory_size = 0;
    std::size_t alignment = 1;

    bool operator==(const SectionLayout&) const = default;
};

class LayoutError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Layout final {
public:
    void build(
        const AssemblyProgram& program,
        const SymbolTable& symbol_table,
        ternary::Word virtual_base = ternary::Word::zero()
    ) {
        sections_.clear();
        symbol_addresses_.clear();
        entry_point_ = ternary::Word::zero();
        has_entry_point_ = false;

        const auto section_sizes = measure_sections(program);

        std::size_t file_offset = 0;
        std::int64_t virtual_address = virtual_base.to_integer();

        constexpr std::string_view order[] = {
            "text",
            "data",
            "rodata",
            "bss"
        };

        for (const auto section_name : order) {
            const auto iterator = section_sizes.find(std::string(section_name));

            if (iterator == section_sizes.end())
                continue;

            const std::size_t memory_size = iterator->second;
            const auto info = section_info(section_name);
            const std::size_t file_size =
                info.type == SectionType::Bss ? 0 : memory_size;

            sections_.push_back(
                SectionLayout{
                    std::string(section_name),
                    info.type,
                    info.flags,
                    file_offset,
                    file_size,
                    ternary::Word::from_integer(virtual_address),
                    memory_size,
                    1
                }
            );

            file_offset += file_size;
            virtual_address += static_cast<std::int64_t>(memory_size);
        }

        resolve_symbol_addresses(symbol_table);

        if (contains_symbol("_start")) {
            entry_point_ = symbol_address("_start");
            has_entry_point_ = true;
        }
    }

    [[nodiscard]] const std::vector<SectionLayout>& sections() const noexcept {
        return sections_;
    }

    [[nodiscard]] const SectionLayout* section(
        std::string_view name
    ) const noexcept {
        for (const auto& value : sections_) {
            if (value.name == name)
                return &value;
        }

        return nullptr;
    }

    [[nodiscard]] bool contains_symbol(std::string_view name) const noexcept {
        return symbol_addresses_.find(std::string(name)) != symbol_addresses_.end();
    }

    [[nodiscard]] ternary::Word symbol_address(std::string_view name) const {
        const auto iterator = symbol_addresses_.find(std::string(name));

        if (iterator == symbol_addresses_.end())
            throw LayoutError(
                "symbol '" + std::string(name) + "' has no layout address"
            );

        return iterator->second;
    }

    [[nodiscard]] bool has_entry_point() const noexcept {
        return has_entry_point_;
    }

    [[nodiscard]] ternary::Word entry_point() const {
        if (!has_entry_point_)
            throw LayoutError("executable has no entry point");

        return entry_point_;
    }

private:
    struct SectionInfo final {
        SectionType type;
        SectionFlags flags;
    };

    [[nodiscard]] static SectionInfo section_info(std::string_view name) {
        if (name == "text")
            return {SectionType::Text, SectionFlags::Executable};

        if (name == "data")
            return {SectionType::Data, SectionFlags::Writable};

        if (name == "rodata")
            return {SectionType::Rodata, SectionFlags::ReadOnly};

        if (name == "bss")
            return {SectionType::Bss, SectionFlags::Writable};

        throw LayoutError(
            "unsupported section '." + std::string(name) + "'"
        );
    }

    [[nodiscard]] static std::unordered_map<std::string, std::size_t>
    measure_sections(const AssemblyProgram& program) {
        std::unordered_map<std::string, std::size_t> sizes;
        std::string current_section;

        for (const auto& statement : program.statements) {
            if (const auto* section = std::get_if<SectionIR>(&statement)) {
                static_cast<void>(section_info(section->name));
                current_section = section->name;
                sizes.try_emplace(current_section, 0);
                continue;
            }

            if (std::holds_alternative<InstructionIR>(statement)) {
                if (current_section.empty())
                    throw LayoutError("instruction encountered outside a section");

                ++sizes[current_section];
            }
        }

        return sizes;
    }

    void resolve_symbol_addresses(const SymbolTable& symbol_table) {
        for (const auto& [name, symbol] : symbol_table.symbols()) {
            const auto* section_layout = section(symbol.section);

            if (section_layout == nullptr)
                throw LayoutError(
                    "symbol '" + name +
                    "' references unknown section '" +
                    symbol.section + "'"
                );

            const std::int64_t address =
                section_layout->virtual_address.to_integer() +
                static_cast<std::int64_t>(symbol.offset);

            symbol_addresses_.emplace(
                name,
                ternary::Word::from_integer(address)
            );
        }
    }

    std::vector<SectionLayout> sections_;
    std::unordered_map<std::string, ternary::Word> symbol_addresses_;
    ternary::Word entry_point_ = ternary::Word::zero();
    bool has_entry_point_ = false;
};

}

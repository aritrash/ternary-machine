#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include <unordered_set>

#include "assembler/ir.hpp"

namespace ternary_machine::assembler {

enum class SymbolBinding {
    Local,
    Global
};

struct Symbol final {
    std::string name;
    std::string section;
    std::size_t offset = 0;
    SymbolBinding binding = SymbolBinding::Local;

    bool operator==(const Symbol&) const = default;
};

class SymbolTableError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class SymbolTable final {
public:
    void build(const AssemblyProgram& program) {
        symbols_.clear();
        globals_.clear();
        references_.clear();

        std::string current_section;
        std::unordered_map<std::string, std::size_t> offsets;

        for (const auto& statement : program.statements) {
            std::visit(
                [&](const auto& value) {
                    process(value, current_section, offsets);
                },
                statement
            );
        }

        for (const auto& reference : references_) {
            if (!contains(reference.name))
                throw error(reference.location, "undefined symbol '" + reference.name + "'");
        }
    }

    [[nodiscard]] bool contains(std::string_view name) const noexcept {
        return symbols_.find(std::string(name)) != symbols_.end();
    }

    [[nodiscard]] const Symbol& lookup(std::string_view name) const {
        const auto iterator = symbols_.find(std::string(name));

        if (iterator == symbols_.end())
            throw SymbolTableError("symbol '" + std::string(name) + "' not found");

        return iterator->second;
    }

    [[nodiscard]] const std::unordered_map<std::string, Symbol>& symbols() const noexcept {
        return symbols_;
    }

private:
    struct SymbolReference final {
        std::string name;
        SourceLocation location{};
    };

    void process(
        const SectionIR& section,
        std::string& current_section,
        std::unordered_map<std::string, std::size_t>& offsets
    ) {
        current_section = section.name;

        if (!offsets.contains(current_section))
            offsets.emplace(current_section, 0);
    }

    void process(
        const GlobalIR& global,
        std::string&,
        std::unordered_map<std::string, std::size_t>&
    ) {
        globals_.insert(global.name);

        const auto iterator = symbols_.find(global.name);

        if (iterator != symbols_.end())
            iterator->second.binding = SymbolBinding::Global;
    }

    void process(
        const LabelIR& label,
        const std::string& current_section,
        std::unordered_map<std::string, std::size_t>& offsets
    ) {
        if (current_section.empty())
            throw error(label.location, "label '" + label.name + "' is outside a section");

        if (symbols_.contains(label.name))
            throw error(label.location, "duplicate symbol '" + label.name + "'");

        const std::size_t offset = offsets[current_section];

        Symbol symbol{
            label.name,
            current_section,
            offset,
            globals_.contains(label.name) ? SymbolBinding::Global : SymbolBinding::Local
        };

        symbols_.emplace(label.name, std::move(symbol));
    }

    void process(
        const InstructionIR& instruction,
        const std::string& current_section,
        std::unordered_map<std::string, std::size_t>& offsets
    ) {
        if (current_section.empty())
            throw error(instruction.location, "instruction '" + instruction.mnemonic + "' is outside a section");

        for (const auto& operand : instruction.operands) {
            if (const auto* symbol = std::get_if<SymbolOperand>(&operand))
                references_.push_back({symbol->name, instruction.location});
        }

        ++offsets[current_section];
    }

    [[nodiscard]] static SymbolTableError error(
        const SourceLocation& location,
        const std::string& message
    ) {
        return SymbolTableError(
            message +
            " at line " + std::to_string(location.line) +
            ", column " + std::to_string(location.column)
        );
    }

    std::unordered_map<std::string, Symbol> symbols_;
    std::unordered_set<std::string> globals_;
    std::vector<SymbolReference> references_;
};

}

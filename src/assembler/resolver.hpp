#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "assembler/ir.hpp"
#include "assembler/symbol_table.hpp"

namespace ternary_machine::assembler {

struct ResolvedSymbol final {
    std::string name;
    std::string section;
    std::size_t offset = 0;
    SymbolBinding binding = SymbolBinding::Local;

    bool operator==(const ResolvedSymbol&) const = default;
};

struct SymbolReference final {
    std::string name;
    std::string section;
    std::size_t instruction_offset = 0;
    ResolvedSymbol target{};

    bool operator==(const SymbolReference&) const = default;
};

class ResolutionError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class SymbolResolver final {
public:
    [[nodiscard]] std::vector<SymbolReference> resolve(
        const AssemblyProgram& program,
        const SymbolTable& symbols
    ) const {
        std::vector<SymbolReference> references;
        std::unordered_map<std::string, std::size_t> offsets;
        std::string current_section;

        for (const auto& statement : program.statements) {
            std::visit(
                [&](const auto& value) {
                    process(value, current_section, offsets, symbols, references);
                },
                statement
            );
        }

        return references;
    }

private:
    void process(
        const SectionIR& section,
        std::string& current_section,
        std::unordered_map<std::string, std::size_t>& offsets,
        const SymbolTable&,
        std::vector<SymbolReference>&
    ) const {
        current_section = section.name;

        if (!offsets.contains(current_section))
            offsets.emplace(current_section, 0);
    }

    void process(
        const GlobalIR&,
        std::string&,
        std::unordered_map<std::string, std::size_t>&,
        const SymbolTable&,
        std::vector<SymbolReference>&
    ) const {}

    void process(
        const LabelIR&,
        std::string&,
        std::unordered_map<std::string, std::size_t>&,
        const SymbolTable&,
        std::vector<SymbolReference>&
    ) const {}

    void process(
        const InstructionIR& instruction,
        const std::string& current_section,
        std::unordered_map<std::string, std::size_t>& offsets,
        const SymbolTable& symbols,
        std::vector<SymbolReference>& references
    ) const {
        if (current_section.empty())
            throw ResolutionError(
                "instruction '" + instruction.mnemonic + "' is outside a section"
            );

        const std::size_t instruction_offset = offsets[current_section];

        for (const auto& operand : instruction.operands) {
            const auto* symbol = std::get_if<SymbolOperand>(&operand);

            if (symbol == nullptr)
                continue;

            if (!symbols.contains(symbol->name))
                throw ResolutionError("undefined symbol '" + symbol->name + "'");

            const auto& target = symbols.lookup(symbol->name);

            references.push_back(SymbolReference{
                symbol->name,
                current_section,
                instruction_offset,
                ResolvedSymbol{
                    target.name,
                    target.section,
                    target.offset,
                    target.binding
                }
            });
        }

        ++offsets[current_section];
    }
};

}

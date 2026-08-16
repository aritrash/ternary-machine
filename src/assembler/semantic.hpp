#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

#include "assembler/ir.hpp"

namespace ternary_machine::assembler {

enum class OperandKind {
    Register,
    Immediate,
    Symbol,
    Memory
};

class SemanticError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class SemanticAnalyzer final {
public:
    void analyze(const AssemblyProgram& program) const {
        for (const auto& statement : program.statements)
            analyze_statement(statement);
    }

private:
    struct InstructionSignature {
        std::string_view mnemonic;
        std::initializer_list<OperandKind> operands;
    };

    static constexpr InstructionSignature INSTRUCTION_SIGNATURES[] = {
        {"NOP", {}},
        {"ADD", {OperandKind::Register, OperandKind::Register, OperandKind::Register}},
        {"SUB", {OperandKind::Register, OperandKind::Register, OperandKind::Register}},
        {"MUL", {OperandKind::Register, OperandKind::Register, OperandKind::Register}},
        {"CMP", {OperandKind::Register, OperandKind::Register}},
        {"TAND", {OperandKind::Register, OperandKind::Register, OperandKind::Register}},
        {"TOR", {OperandKind::Register, OperandKind::Register, OperandKind::Register}},
        {"TXOR", {OperandKind::Register, OperandKind::Register, OperandKind::Register}},
        {"TNOT", {OperandKind::Register, OperandKind::Register}},
        {"SHF", {OperandKind::Register, OperandKind::Register, OperandKind::Register}},
        {"LDI", {OperandKind::Register, OperandKind::Immediate}},
        {"MOV", {OperandKind::Register, OperandKind::Register}},
        {"LD", {OperandKind::Register, OperandKind::Memory}},
        {"ST", {OperandKind::Memory, OperandKind::Register}},
        {"LEA", {OperandKind::Register, OperandKind::Memory}},
        {"JMP", {OperandKind::Symbol}},
        {"BEQ", {OperandKind::Symbol}},
        {"BGT", {OperandKind::Symbol}},
        {"BLT", {OperandKind::Symbol}},
        {"CALL", {OperandKind::Symbol}},
        {"RET", {}},
        {"IN", {OperandKind::Register, OperandKind::Immediate}},
        {"OUT", {OperandKind::Register, OperandKind::Immediate}},
        {"SYS", {}},
        {"IRET", {}},
        {"SWAP", {}},
        {"HLT", {}}
    };

    void analyze_statement(const Statement& statement) const {
        std::visit([this](const auto& value) { analyze_value(value); }, statement);
    }

    void analyze_value(const InstructionIR& instruction) const {
        const auto* signature = find_instruction(instruction.mnemonic);

        if (signature == nullptr)
            throw error(instruction.location, "unknown instruction '" + instruction.mnemonic + "'");

        if (instruction.operands.size() != signature->operands.size()) {
            throw error(
                instruction.location,
                "instruction '" + instruction.mnemonic + "' expects " +
                std::to_string(signature->operands.size()) + " operand" +
                (signature->operands.size() == 1 ? "" : "s") + ", got " +
                std::to_string(instruction.operands.size())
            );
        }

        for (std::size_t i = 0; i < instruction.operands.size(); ++i) {
            const OperandKind actual = operand_kind(instruction.operands[i]);
            const OperandKind expected = signature->operands.begin()[i];

            if (actual != expected) {
                throw error(
                    instruction.location,
                    "operand " + std::to_string(i + 1) + " of '" +
                    instruction.mnemonic + "' has the wrong type"
                );
            }

            validate_operand(instruction.operands[i], instruction.location);
        }
    }

    void analyze_value(const LabelIR& label) const {
        validate_symbol_name(label.name, label.location, "label");
    }

    void analyze_value(const SectionIR& section) const {
        if (section.name.empty())
            throw error(section.location, "section name cannot be empty");

        if (!is_valid_identifier(section.name))
            throw error(section.location, "invalid section name '" + section.name + "'");
    }

    void analyze_value(const GlobalIR& global) const {
        validate_symbol_name(global.name, global.location, "global symbol");
    }

    [[nodiscard]] static const InstructionSignature* find_instruction(std::string_view mnemonic) noexcept {
        for (const auto& signature : INSTRUCTION_SIGNATURES)
            if (signature.mnemonic == mnemonic)
                return &signature;

        return nullptr;
    }

    [[nodiscard]] static OperandKind operand_kind(const Operand& operand) noexcept {
        return std::visit([](const auto& value) -> OperandKind {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, RegisterOperand>)
                return OperandKind::Register;
            else if constexpr (std::is_same_v<T, ImmediateOperand>)
                return OperandKind::Immediate;
            else if constexpr (std::is_same_v<T, SymbolOperand>)
                return OperandKind::Symbol;
            else
                return OperandKind::Memory;
        }, operand);
    }

    static void validate_operand(const Operand& operand, const SourceLocation& location) {
        if (const auto* value = std::get_if<RegisterOperand>(&operand)) {
            if (value->index > 8)
                throw error(location, "invalid TVM register R" + std::to_string(value->index));
        }
    }

    static void validate_symbol_name(const std::string& name, const SourceLocation& location, const char* kind) {
        if (!is_valid_identifier(name))
            throw error(location, "invalid " + std::string(kind) + " name '" + name + "'");
    }

    [[nodiscard]] static bool is_valid_identifier(std::string_view value) noexcept {
        if (value.empty())
            return false;

        const auto is_start = [](char c) {
            return (c >= 'A' && c <= 'Z') ||
                   (c >= 'a' && c <= 'z') ||
                   c == '_';
        };

        const auto is_body = [&](char c) {
            return is_start(c) ||
                   (c >= '0' && c <= '9');
        };

        if (!is_start(value.front()))
            return false;

        for (std::size_t i = 1; i < value.size(); ++i)
            if (!is_body(value[i]))
                return false;

        return true;
    }

    [[nodiscard]] static SemanticError error(const SourceLocation& location, const std::string& message) {
        return SemanticError(
            message +
            " at line " + std::to_string(location.line) +
            ", column " + std::to_string(location.column)
        );
    }
};

}

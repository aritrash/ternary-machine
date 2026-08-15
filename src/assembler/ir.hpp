#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace ternary_machine::assembler {

struct SourceLocation final {
    std::size_t line = 1;
    std::size_t column = 1;

    bool operator==(const SourceLocation&) const = default;
};

struct RegisterOperand final {
    std::uint8_t index = 0;

    bool operator==(const RegisterOperand&) const = default;
};

struct ImmediateOperand final {
    std::int64_t value = 0;

    bool operator==(const ImmediateOperand&) const = default;
};

struct SymbolOperand final {
    std::string name;

    bool operator==(const SymbolOperand&) const = default;
};

struct MemoryOperand final {
    std::uint8_t base = 0;
    std::int64_t offset = 0;

    bool operator==(const MemoryOperand&) const = default;
};

using Operand = std::variant<RegisterOperand, ImmediateOperand, SymbolOperand, MemoryOperand>;

struct InstructionIR final {
    std::string mnemonic;
    std::vector<Operand> operands;
    SourceLocation location{};

    bool operator==(const InstructionIR&) const = default;
};

struct LabelIR final {
    std::string name;
    SourceLocation location{};

    bool operator==(const LabelIR&) const = default;
};

struct SectionIR final {
    std::string name;
    SourceLocation location{};

    bool operator==(const SectionIR&) const = default;
};

struct GlobalIR final {
    std::string name;
    SourceLocation location{};

    bool operator==(const GlobalIR&) const = default;
};

using Statement = std::variant<InstructionIR, LabelIR, SectionIR, GlobalIR>;

struct AssemblyProgram final {
    std::vector<Statement> statements;

    bool operator==(const AssemblyProgram&) const = default;
};

}

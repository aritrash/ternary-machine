#include <cassert>
#include <string>

#include "assembler/lexer.hpp"
#include "assembler/parser.hpp"
#include "assembler/resolver.hpp"
#include "assembler/semantic.hpp"
#include "assembler/symbol_table.hpp"

using ternary_machine::assembler::Lexer;
using ternary_machine::assembler::Parser;
using ternary_machine::assembler::ResolutionError;
using ternary_machine::assembler::SemanticAnalyzer;
using ternary_machine::assembler::SymbolBinding;
using ternary_machine::assembler::SymbolResolver;
using ternary_machine::assembler::SymbolTable;

static std::pair<ternary_machine::assembler::AssemblyProgram, SymbolTable> prepare(const char* source) {
    const auto tokens = Lexer(source).tokenize();
    const auto program = Parser(tokens).parse();
    SemanticAnalyzer{}.analyze(program);

    SymbolTable symbols;
    symbols.build(program);

    return {program, std::move(symbols)};
}

static std::vector<ternary_machine::assembler::SymbolReference> resolve(const char* source) {
    auto [program, symbols] = prepare(source);
    return SymbolResolver{}.resolve(program, symbols);
}

static bool rejects(const char* source) {
    try {
        (void)resolve(source);
    } catch (const ResolutionError&) {
        return true;
    }

    return false;
}

int main() {
    {
        const auto references = resolve(
            "section .text\n"
            "_start:\n"
            "JMP loop\n"
            "loop:\n"
            "HLT\n"
        );

        assert(references.size() == 1);

        const auto& reference = references[0];

        assert(reference.name == "loop");
        assert(reference.section == "text");
        assert(reference.instruction_offset == 0);
        assert(reference.target.name == "loop");
        assert(reference.target.section == "text");
        assert(reference.target.offset == 1);
        assert(reference.target.binding == SymbolBinding::Local);
    }

    {
        const auto references = resolve(
            "section .text\n"
            "JMP loop\n"
            "LDI R1, 42\n"
            "loop:\n"
            "HLT\n"
        );

        assert(references.size() == 1);
        assert(references[0].name == "loop");
        assert(references[0].instruction_offset == 0);
        assert(references[0].target.offset == 2);
    }

    {
        const auto references = resolve(
            "section .text\n"
            "global _start\n"
            "_start:\n"
            "CALL function\n"
            "HLT\n"
            "function:\n"
            "RET\n"
        );

        assert(references.size() == 1);
        assert(references[0].name == "function");
        assert(references[0].instruction_offset == 0);
        assert(references[0].target.offset == 2);
        assert(references[0].target.binding == SymbolBinding::Local);
    }

    {
        const auto references = resolve(
            "section .text\n"
            "start:\n"
            "BEQ equal\n"
            "BGT greater\n"
            "BLT less\n"
            "JMP done\n"
            "equal:\n"
            "HLT\n"
            "greater:\n"
            "HLT\n"
            "less:\n"
            "HLT\n"
            "done:\n"
            "RET\n"
        );

        assert(references.size() == 4);

        assert(references[0].name == "equal");
        assert(references[0].instruction_offset == 0);
        assert(references[0].target.offset == 4);

        assert(references[1].name == "greater");
        assert(references[1].instruction_offset == 1);
        assert(references[1].target.offset == 5);

        assert(references[2].name == "less");
        assert(references[2].instruction_offset == 2);
        assert(references[2].target.offset == 6);

        assert(references[3].name == "done");
        assert(references[3].instruction_offset == 3);
        assert(references[3].target.offset == 7);
    }

    {
        const auto references = resolve(
            "section .text\n"
            "start:\n"
            "JMP data_label\n"
            "section .data\n"
            "data_label:\n"
            "HLT\n"
        );

        assert(references.size() == 1);
        assert(references[0].name == "data_label");
        assert(references[0].section == "text");
        assert(references[0].instruction_offset == 0);
        assert(references[0].target.section == "data");
        assert(references[0].target.offset == 0);
    }

    {
        assert(rejects(
            "section .text\n"
            "JMP missing\n"
        ));
    }

    {
        assert(rejects(
            "section .text\n"
            "CALL missing_function\n"
        ));
    }

    return 0;
}

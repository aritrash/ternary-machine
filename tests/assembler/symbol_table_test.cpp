#include <cassert>
#include <string>

#include "assembler/lexer.hpp"
#include "assembler/parser.hpp"
#include "assembler/semantic.hpp"
#include "assembler/symbol_table.hpp"

using ternary_machine::assembler::Lexer;
using ternary_machine::assembler::Parser;
using ternary_machine::assembler::SemanticAnalyzer;
using ternary_machine::assembler::SymbolBinding;
using ternary_machine::assembler::SymbolTable;
using ternary_machine::assembler::SymbolTableError;

static SymbolTable build(const char* source) {
    const auto tokens = Lexer(source).tokenize();
    const auto program = Parser(tokens).parse();
    SemanticAnalyzer{}.analyze(program);

    SymbolTable table;
    table.build(program);
    return table;
}

static bool rejects(const char* source) {
    try {
        build(source);
    } catch (const SymbolTableError&) {
        return true;
    }

    return false;
}

int main() {
    {
        const auto table = build(
            "section .text\n"
            "_start:\n"
            "HLT\n"
        );

        assert(table.contains("_start"));

        const auto& symbol = table.lookup("_start");

        assert(symbol.name == "_start");
        assert(symbol.section == "text");
        assert(symbol.offset == 0);
        assert(symbol.binding == SymbolBinding::Local);
    }

    {
        const auto table = build(
            "section .text\n"
            "_start:\n"
            "LDI R1, 10\n"
            "LDI R2, 20\n"
            "loop:\n"
            "ADD R3, R1, R2\n"
            "JMP loop\n"
        );

        assert(table.lookup("_start").offset == 0);
        assert(table.lookup("loop").offset == 2);
    }

    {
        const auto table = build(
            "section .text\n"
            "global _start\n"
            "_start:\n"
            "HLT\n"
        );

        assert(table.lookup("_start").binding == SymbolBinding::Global);
    }

    {
        const auto table = build(
            "section .text\n"
            "_start:\n"
            "HLT\n"
            "global _start\n"
        );

        assert(table.lookup("_start").binding == SymbolBinding::Global);
    }

    {
        const auto table = build(
            "section .text\n"
            "_start:\n"
            "JMP loop\n"
            "LDI R1, 42\n"
            "loop:\n"
            "HLT\n"
        );

        assert(table.lookup("_start").offset == 0);
        assert(table.lookup("loop").offset == 2);
    }

    {
        const auto table = build(
            "section .text\n"
            "first:\n"
            "HLT\n"
            "section .data\n"
            "second:\n"
            "HLT\n"
        );

        assert(table.lookup("first").section == "text");
        assert(table.lookup("first").offset == 0);

        assert(table.lookup("second").section == "data");
        assert(table.lookup("second").offset == 0);
    }

    {
        const auto table = build(
            "section .text\n"
            "first:\n"
            "HLT\n"
            "section .data\n"
            "second:\n"
            "HLT\n"
            "section .text\n"
            "third:\n"
            "HLT\n"
        );

        assert(table.lookup("first").offset == 0);
        assert(table.lookup("second").offset == 0);
        assert(table.lookup("third").offset == 1);
    }

    {
        const auto table = build(
            "section .text\n"
            "global main\n"
            "main:\n"
            "CALL function\n"
            "HLT\n"
            "function:\n"
            "RET\n"
        );

        assert(table.lookup("main").binding == SymbolBinding::Global);
        assert(table.lookup("main").offset == 0);
        assert(table.lookup("function").binding == SymbolBinding::Local);
        assert(table.lookup("function").offset == 2);
    }

    {
        assert(rejects(
            "section .text\n"
            "start:\n"
            "HLT\n"
            "start:\n"
            "HLT\n"
        ));
    }

    {
        const auto table = build(
            "section .text\n"
            "JMP missing\n"
            "HLT\n"
        );

        assert(table.symbols().empty());
        assert(!table.contains("missing"));
    }

    {
        assert(rejects(
            "start:\n"
            "HLT\n"
        ));
    }

    return 0;
}

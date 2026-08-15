#include <cassert>

#include "assembler/layout.hpp"
#include "assembler/lexer.hpp"
#include "assembler/parser.hpp"
#include "assembler/semantic.hpp"
#include "assembler/symbol_table.hpp"

using ternary_machine::assembler::Layout;
using ternary_machine::assembler::LayoutError;
using ternary_machine::assembler::Lexer;
using ternary_machine::assembler::Parser;
using ternary_machine::assembler::SectionFlags;
using ternary_machine::assembler::SectionType;
using ternary_machine::assembler::SemanticAnalyzer;
using ternary_machine::assembler::SymbolTable;
using ternary_machine::assembler::has_flag;
using ternary_machine::ternary::Word;

static Layout build(
    const char* source,
    Word virtual_base = Word::zero()
) {
    const auto tokens = Lexer(source).tokenize();
    const auto program = Parser(tokens).parse();

    SemanticAnalyzer{}.analyze(program);

    SymbolTable symbols;
    symbols.build(program);

    Layout layout;
    layout.build(program, symbols, virtual_base);

    return layout;
}

int main() {
    {
        const auto layout = build(
            "section .text\n"
            "_start:\n"
            "LDI R1, 42\n"
            "HLT\n"
        );

        assert(layout.sections().size() == 1);

        const auto* text = layout.section("text");

        assert(text != nullptr);
        assert(text->type == SectionType::Text);
        assert(has_flag(text->flags, SectionFlags::Executable));
        assert(!has_flag(text->flags, SectionFlags::Writable));
        assert(text->file_offset == 0);
        assert(text->file_size == 2);
        assert(text->memory_size == 2);
        assert(text->virtual_address == Word::zero());
        assert(text->alignment == 1);

        assert(layout.contains_symbol("_start"));
        assert(layout.symbol_address("_start") == Word::zero());

        assert(layout.has_entry_point());
        assert(layout.entry_point() == Word::zero());
    }

    {
        const auto layout = build(
            "section .text\n"
            "_start:\n"
            "LDI R1, 10\n"
            "LDI R2, 20\n"
            "loop:\n"
            "ADD R3, R1, R2\n"
            "JMP loop\n"
        );

        assert(layout.symbol_address("_start").to_integer() == 0);
        assert(layout.symbol_address("loop").to_integer() == 2);
    }

    {
        const auto layout = build(
            "section .data\n"
            "data_value:\n"
            "HLT\n"
            "section .rodata\n"
            "constant:\n"
            "HLT\n"
            "section .text\n"
            "_start:\n"
            "HLT\n"
        );

        assert(layout.sections().size() == 3);

        assert(layout.sections()[0].name == "text");
        assert(layout.sections()[1].name == "data");
        assert(layout.sections()[2].name == "rodata");

        const auto* text = layout.section("text");
        const auto* data = layout.section("data");
        const auto* rodata = layout.section("rodata");

        assert(text != nullptr);
        assert(data != nullptr);
        assert(rodata != nullptr);

        assert(text->virtual_address.to_integer() == 0);
        assert(data->virtual_address.to_integer() == 1);
        assert(rodata->virtual_address.to_integer() == 2);

        assert(text->file_offset == 0);
        assert(data->file_offset == 1);
        assert(rodata->file_offset == 2);

        assert(layout.symbol_address("_start").to_integer() == 0);
        assert(layout.symbol_address("data_value").to_integer() == 1);
        assert(layout.symbol_address("constant").to_integer() == 2);
    }

    {
        const auto layout = build(
            "section .text\n"
            "_start:\n"
            "HLT\n"
            "section .bss\n"
            "buffer:\n"
            "HLT\n"
        );

        const auto* text = layout.section("text");
        const auto* bss = layout.section("bss");

        assert(text != nullptr);
        assert(bss != nullptr);

        assert(bss->type == SectionType::Bss);
        assert(has_flag(bss->flags, SectionFlags::Writable));
        assert(bss->file_offset == 1);
        assert(bss->file_size == 0);
        assert(bss->memory_size == 1);
        assert(bss->virtual_address.to_integer() == 1);

        assert(layout.symbol_address("buffer").to_integer() == 1);
    }

    {
        const auto layout = build(
            "section .text\n"
            "_start:\n"
            "HLT\n",
            Word::from_integer(100)
        );

        const auto* text = layout.section("text");

        assert(text != nullptr);
        assert(text->virtual_address.to_integer() == 100);
        assert(layout.symbol_address("_start").to_integer() == 100);
        assert(layout.entry_point().to_integer() == 100);
    }

    {
        const auto layout = build(
            "section .text\n"
            "main:\n"
            "HLT\n"
        );

        assert(!layout.has_entry_point());

        bool threw = false;

        try {
            static_cast<void>(layout.entry_point());
        } catch (const LayoutError&) {
            threw = true;
        }

        assert(threw);
    }

    {
        const auto layout = build(
            "section .text\n"
            "_start:\n"
            "HLT\n"
            "section .data\n"
            "value:\n"
            "HLT\n"
            "section .bss\n"
            "buffer:\n"
            "HLT\n"
        );

        const auto* text = layout.section("text");
        const auto* data = layout.section("data");
        const auto* bss = layout.section("bss");

        assert(text != nullptr);
        assert(data != nullptr);
        assert(bss != nullptr);

        assert(text->file_offset == 0);
        assert(data->file_offset == 1);
        assert(bss->file_offset == 2);

        assert(text->file_size == 1);
        assert(data->file_size == 1);
        assert(bss->file_size == 0);

        assert(text->virtual_address.to_integer() == 0);
        assert(data->virtual_address.to_integer() == 1);
        assert(bss->virtual_address.to_integer() == 2);
    }

    return 0;
}

#include <cassert>
#include <cstdint>
#include <functional>
#include <stdexcept>

#include "assembler/assembler.hpp"
#include "assembler/trn_format.hpp"
#include "assembler/trn_image.hpp"
#include "ternary/instruction.hpp"
#include "ternary/word.hpp"

using ternary_machine::assembler::Assembler;
using ternary_machine::assembler::TrnFormat;
using ternary_machine::assembler::TrnImage;
using ternary_machine::ternary::Instruction;
using ternary_machine::ternary::Opcode;
using ternary_machine::ternary::Trit;
using ternary_machine::ternary::Word;

static bool rejects(const std::function<void()>& function) {
    try {
        function();
    } catch (const std::exception&) {
        return true;
    }

    return false;
}

static Word decode_image_word(const TrnImage::Word& encoded) {
    Word word;

    for (std::size_t i = 0; i < encoded.size(); ++i) {
        switch (encoded[i]) {
            case 'n':
                word.set_trit(i, Trit::Neg);
                break;
            case '0':
                word.set_trit(i, Trit::Zero);
                break;
            case '1':
                word.set_trit(i, Trit::Pos);
                break;
            default:
                throw std::runtime_error("invalid encoded trit");
        }
    }

    return word;
}

static const TrnFormat::SectionEntry* find_section(
    const TrnImage& image,
    TrnFormat::SectionType type
) {
    for (const auto& section : image.sections())
        if (section.type == type)
            return &section;

    return nullptr;
}

int main() {
    /*
     * Basic executable generation.
     *
     * _start may legitimately receive symbol identifier 0, so the test
     * verifies that the header identifier refers to a valid ENTRY symbol
     * instead of assuming that the identifier must be non-zero.
     */
    {
        Assembler assembler;

        const auto image = assembler.assemble(
            "section .text\n"
            "global _start\n"
            "_start:\n"
            "HLT\n"
        );

        assert(image.is_laid_out());
        assert(image.text().size() == 1);
        assert(image.header().section_count == 1);
        assert(image.header().symbol_count >= 1);
        assert(image.header().entry_point == 0);

        const auto start_symbol = image.header().start_symbol;

        assert(start_symbol >= 0);
        assert(static_cast<std::size_t>(start_symbol) < image.symbols().size());

        const auto& entry = image.symbols()[static_cast<std::size_t>(start_symbol)];

        assert(entry.identifier == start_symbol);
        assert(entry.type == TrnFormat::SymbolType::Entry);
        assert(entry.offset == 0);
    }

    /*
     * Instruction encoding must survive the complete source -> image path.
     */
    {
        Assembler assembler;

        const auto image = assembler.assemble(
            "section .text\n"
            "_start:\n"
            "LDI R1, 42\n"
            "HLT\n"
        );

        assert(image.text().size() == 2);

        const auto first = decode_image_word(image.text()[0]);
        const auto instruction = Instruction::decode(first);

        assert(instruction.opcode() == Opcode::LDI);
        assert(instruction.rd() == 1);
        assert(instruction.immediate() == 42);
    }

    /*
     * Forward branch resolution.
     */
    {
        Assembler assembler;

        const auto image = assembler.assemble(
            "section .text\n"
            "_start:\n"
            "JMP loop\n"
            "HLT\n"
            "loop:\n"
            "HLT\n"
        );

        assert(image.text().size() == 3);

        const auto instruction = Instruction::decode(
            decode_image_word(image.text()[0])
        );

        assert(instruction.opcode() == Opcode::JMP);
        assert(instruction.immediate() == 2);
    }

    /*
     * CALL target resolution.
     */
    {
        Assembler assembler;

        const auto image = assembler.assemble(
            "section .text\n"
            "_start:\n"
            "CALL function\n"
            "HLT\n"
            "function:\n"
            "RET\n"
        );

        assert(image.text().size() == 3);

        const auto instruction = Instruction::decode(
            decode_image_word(image.text()[0])
        );

        assert(instruction.opcode() == Opcode::CALL);
        assert(instruction.immediate() == 2);
    }

    /*
     * Section creation and BSS representation.
     *
     * BSS has no physical payload, so its file size remains zero.
     */
    {
        Assembler assembler;

        const auto image = assembler.assemble(
            "section .text\n"
            "_start:\n"
            "HLT\n"
            "section .data\n"
            "section .rodata\n"
            "section .bss\n"
        );

        assert(image.sections().size() == 4);

        const auto* text = find_section(image, TrnFormat::SectionType::Text);
        const auto* data = find_section(image, TrnFormat::SectionType::Data);
        const auto* rodata = find_section(image, TrnFormat::SectionType::Rodata);
        const auto* bss = find_section(image, TrnFormat::SectionType::Bss);

        assert(text != nullptr);
        assert(data != nullptr);
        assert(rodata != nullptr);
        assert(bss != nullptr);

        assert(text->size == 1);
        assert(data->size == 0);
        assert(rodata->size == 0);
        assert(bss->size == 0);

        assert(bss->file_offset == 0);
    }

    /*
     * Text section layout.
     */
    {
        Assembler assembler;

        const auto image = assembler.assemble(
            "section .text\n"
            "_start:\n"
            "HLT\n"
            "HLT\n"
            "HLT\n"
        );

        const auto* text = find_section(image, TrnFormat::SectionType::Text);

        assert(text != nullptr);
        assert(text->size == 3);
        assert(
            text->file_offset >=
            static_cast<std::int64_t>(TrnFormat::HeaderWords)
        );
        assert(text->virtual_address == 0);
        assert(text->alignment == 1);
    }

    /*
     * Entry symbol must be represented consistently in the symbol table
     * and referenced by the executable header.
     */
    {
        Assembler assembler;

        const auto image = assembler.assemble(
            "section .text\n"
            "global _start\n"
            "_start:\n"
            "HLT\n"
        );

        assert(image.header().entry_point == 0);

        const auto start_symbol = image.header().start_symbol;

        assert(start_symbol >= 0);
        assert(static_cast<std::size_t>(start_symbol) < image.symbols().size());

        const auto& entry = image.symbols()[static_cast<std::size_t>(start_symbol)];

        assert(entry.identifier == start_symbol);
        assert(entry.type == TrnFormat::SymbolType::Entry);
        assert(entry.offset == 0);

        bool found_start = false;

        for (const auto& symbol : image.symbols()) {
            if (symbol.identifier == start_symbol) {
                found_start = true;
                assert(symbol.offset == 0);
                assert(symbol.type == TrnFormat::SymbolType::Entry);
                break;
            }
        }

        assert(found_start);
    }

    /*
     * Local labels must survive into the generated symbol table with their
     * correct section-relative offsets.
     */
    {
        Assembler assembler;

        const auto image = assembler.assemble(
            "section .text\n"
            "_start:\n"
            "first:\n"
            "HLT\n"
            "second:\n"
            "HLT\n"
            "third:\n"
            "HLT\n"
        );

        assert(image.symbols().size() >= 4);

        bool found_first = false;
        bool found_second = false;
        bool found_third = false;

        for (const auto& symbol : image.symbols()) {
            if (symbol.offset == 0 && symbol.type != TrnFormat::SymbolType::Entry)
                found_first = true;

            if (symbol.offset == 1)
                found_second = true;

            if (symbol.offset == 2)
                found_third = true;
        }

        assert(found_first);
        assert(found_second);
        assert(found_third);
    }

    /*
     * Undefined branch target must be rejected.
     */
    {
        Assembler assembler;

        assert(rejects([&] {
            static_cast<void>(assembler.assemble(
                "section .text\n"
                "_start:\n"
                "JMP missing\n"
            ));
        }));
    }

    /*
     * Unknown instruction must be rejected.
     */
    {
        Assembler assembler;

        assert(rejects([&] {
            static_cast<void>(assembler.assemble(
                "section .text\n"
                "_start:\n"
                "INVALID R1, R2\n"
            ));
        }));
    }

    /*
     * Invalid register must be rejected.
     */
    {
        Assembler assembler;

        assert(rejects([&] {
            static_cast<void>(assembler.assemble(
                "section .text\n"
                "_start:\n"
                "LDI R9, 42\n"
            ));
        }));
    }

    /*
     * Invalid operand count must be rejected.
     */
    {
        Assembler assembler;

        assert(rejects([&] {
            static_cast<void>(assembler.assemble(
                "section .text\n"
                "_start:\n"
                "ADD R1, R2\n"
            ));
        }));
    }

    /*
     * Duplicate labels must be rejected.
     */
    {
        Assembler assembler;

        assert(rejects([&] {
            static_cast<void>(assembler.assemble(
                "section .text\n"
                "_start:\n"
                "first:\n"
                "HLT\n"
                "first:\n"
                "HLT\n"
            ));
        }));
    }

    /*
     * An executable without _start must be rejected.
     *
     * This is intentionally a rejection test. It must not inspect
     * header().start_symbol because no executable image is expected.
     */
    {
        Assembler assembler;

        assert(rejects([&] {
            static_cast<void>(assembler.assemble(
                "section .text\n"
                "HLT\n"
            ));
        }));
    }

    /*
     * Branch resolution must account for instructions between the branch
     * and its target.
     */
    {
        Assembler assembler;

        const auto image = assembler.assemble(
            "section .text\n"
            "_start:\n"
            "LDI R1, 10\n"
            "JMP done\n"
            "LDI R1, 99\n"
            "done:\n"
            "HLT\n"
        );

        assert(image.text().size() == 4);

        const auto jump = Instruction::decode(
            decode_image_word(image.text()[1])
        );

        assert(jump.opcode() == Opcode::JMP);
        assert(jump.immediate() == 2);
    }

    return 0;
}

#include <cassert>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>

#include "assembler/assembler.hpp"
#include "assembler/trn_writer.hpp"
#include "runtime/trn_loader.hpp"
#include "ternary/instruction.hpp"

using ternary_machine::assembler::Assembler;
using ternary_machine::assembler::TrnWriter;
using ternary_machine::runtime::LoadedImage;
using ternary_machine::runtime::TrnLoader;
using ternary_machine::runtime::TrnLoaderError;
using ternary_machine::ternary::Instruction;
using ternary_machine::ternary::Opcode;
using ternary_machine::ternary::Word;

static bool rejects(const std::function<void()>& function) {
    try {
        function();
    } catch (const TrnLoaderError&) {
        return true;
    }

    return false;
}

static std::filesystem::path temporary_path(const char* name) {
    return std::filesystem::temp_directory_path() / name;
}

static void write_image(const std::filesystem::path& path, const char* source) {
    Assembler assembler;
    const auto image = assembler.assemble(source);
    TrnWriter{}.write(image, path.string());
}

int main() {
    const auto path = temporary_path("ternary_machine_loader_test.trn");

    {
        write_image(
            path,
            "section .text\n"
            "global _start\n"
            "_start:\n"
            "LDI R1, 42\n"
            "HLT\n"
        );

        const auto image = TrnLoader{}.load(path.string());

        assert(image.architecture_id() == 1);
        assert(image.isa_version() == 1);
        assert(image.format_version() == 1);
        assert(image.entry_point() == 0);
        assert(image.start_symbol() >= 0);
        assert(image.is_valid_entry_point());

        assert(image.text() != nullptr);
        assert(image.text()->memory_size == 2);
        assert(image.text()->words.size() == 2);
        assert(image.text()->readable());
        assert(image.text()->executable());
        assert(!image.text()->writable());

        const auto first = Instruction::decode(image.text()->words[0]);

        assert(first.opcode() == Opcode::LDI);
        assert(first.rd() == 1);
        assert(first.immediate() == 42);

        const auto second = Instruction::decode(image.text()->words[1]);

        assert(second.opcode() == Opcode::HLT);
    }

    {
        write_image(
            path,
            "section .text\n"
            "_start:\n"
            "HLT\n"
            "section .data\n"
            "section .rodata\n"
            "section .bss\n"
        );

        const auto image = TrnLoader{}.load(path.string());

        assert(image.has_section(LoadedImage::SectionType::Text));
        assert(image.has_section(LoadedImage::SectionType::Data));
        assert(image.has_section(LoadedImage::SectionType::Rodata));
        assert(image.has_section(LoadedImage::SectionType::Bss));

        assert(image.data() != nullptr);
        assert(image.data()->words.empty());

        assert(image.rodata() != nullptr);
        assert(image.rodata()->words.empty());

        assert(image.bss() != nullptr);
        assert(image.bss()->memory_size == 0);
        assert(image.bss()->words.empty());
        assert(!image.bss()->has_payload());
    }

    {
        write_image(
            path,
            "section .text\n"
            "_start:\n"
            "JMP loop\n"
            "HLT\n"
            "loop:\n"
            "HLT\n"
        );

        const auto image = TrnLoader{}.load(path.string());

        assert(image.text() != nullptr);
        assert(image.text()->words.size() == 3);

        const auto instruction = Instruction::decode(image.text()->words[0]);

        assert(instruction.opcode() == Opcode::JMP);
        assert(instruction.immediate() == 2);

        assert(image.symbols().size() >= 2);
    }

    {
        write_image(
            path,
            "section .text\n"
            "global _start\n"
            "_start:\n"
            "HLT\n"
        );

        const auto image = TrnLoader{}.load(path.string());

        const auto start = image.start_symbol();

        assert(start >= 0);
        assert(static_cast<std::size_t>(start) < image.symbols().size());

        const auto& symbol = image.symbols()[static_cast<std::size_t>(start)];

        assert(symbol.identifier == start);
        assert(symbol.type == 2);
        assert(symbol.section == 0);
        assert(symbol.address == image.entry_point());
    }

    {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);

        assert(output);

        const char invalid[] = "not a ternary executable";
        output.write(invalid, sizeof(invalid) - 1);
    }

    assert(rejects([&] {
        static_cast<void>(TrnLoader{}.load(path.string()));
    }));

    {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);

        assert(output);

        for (std::size_t i = 0; i < 16 * 27; ++i)
            output.put('0');
    }

    assert(rejects([&] {
        static_cast<void>(TrnLoader{}.load(path.string()));
    }));

    {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);

        assert(output);

        for (std::size_t i = 0; i < 16 * 27; ++i)
            output.put('0');

        output.put('x');
    }

    assert(rejects([&] {
        static_cast<void>(TrnLoader{}.load(path.string()));
    }));

    {
        write_image(
            path,
            "section .text\n"
            "_start:\n"
            "HLT\n"
        );

        const auto image = TrnLoader{}.load(path.string());

        assert(image.text() != nullptr);
        assert(image.text()->virtual_address == image.memory_base());
        assert(image.entry_point() == image.text()->virtual_address);
        assert(image.is_valid_entry_point());
    }

    std::filesystem::remove(path);

    return 0;
}

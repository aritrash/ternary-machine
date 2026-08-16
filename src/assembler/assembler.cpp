#include "assembler/assembler.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "assembler/encoder.hpp"
#include "assembler/layout.hpp"
#include "assembler/lexer.hpp"
#include "assembler/parser.hpp"
#include "assembler/resolver.hpp"
#include "assembler/semantic.hpp"
#include "assembler/symbol_table.hpp"
#include "assembler/trn_writer.hpp"

namespace ternary_machine::assembler {

namespace {

[[nodiscard]] bool is_branch(std::string_view mnemonic) noexcept {
    return mnemonic == "JMP" ||
           mnemonic == "BEQ" ||
           mnemonic == "BGT" ||
           mnemonic == "BLT" ||
           mnemonic == "CALL";
}

[[nodiscard]] TrnFormat::SectionType format_section_type(SectionType type) {
    switch (type) {
        case SectionType::Text:
            return TrnFormat::SectionType::Text;
        case SectionType::Data:
            return TrnFormat::SectionType::Data;
        case SectionType::Rodata:
            return TrnFormat::SectionType::Rodata;
        case SectionType::Bss:
            return TrnFormat::SectionType::Bss;
    }

    throw LayoutError("invalid section type");
}

[[nodiscard]] std::int64_t format_section_flags(SectionType type) {
    switch (type) {
        case SectionType::Text:
            return static_cast<std::int64_t>(TrnFormat::SectionFlag::Read) |
                   static_cast<std::int64_t>(TrnFormat::SectionFlag::Execute);

        case SectionType::Data:
            return static_cast<std::int64_t>(TrnFormat::SectionFlag::Read) |
                   static_cast<std::int64_t>(TrnFormat::SectionFlag::Write);

        case SectionType::Rodata:
            return static_cast<std::int64_t>(TrnFormat::SectionFlag::Read);

        case SectionType::Bss:
            return static_cast<std::int64_t>(TrnFormat::SectionFlag::Read) |
                   static_cast<std::int64_t>(TrnFormat::SectionFlag::Write);
    }

    throw LayoutError("invalid section type");
}

[[nodiscard]] std::size_t section_index(const Layout& layout, std::string_view name) {
    const auto& sections = layout.sections();

    for (std::size_t i = 0; i < sections.size(); ++i)
        if (sections[i].name == name)
            return i;

    throw LayoutError("section '." + std::string(name) + "' is missing from layout");
}

[[nodiscard]] TrnFormat::Word serialize_word(const ternary::Word& word) {
    TrnFormat::Word result{};
    const auto encoded = word.to_string();

    if (encoded.size() != TrnFormat::WordTrits)
        throw TrnFormatError("ternary word has an invalid width");

    std::copy(encoded.begin(), encoded.end(), result.begin());
    return result;
}

[[nodiscard]] InstructionIR resolve_branch(
    const InstructionIR& instruction,
    std::size_t instruction_offset,
    const SectionLayout& section,
    const Layout& layout
) {
    if (!is_branch(instruction.mnemonic))
        return instruction;

    if (instruction.operands.size() != 1)
        throw EncodingError("incorrect operand count for " + instruction.mnemonic);

    const auto* symbol = std::get_if<SymbolOperand>(&instruction.operands[0]);

    if (symbol == nullptr)
        throw EncodingError("branch target must be a symbol");

    if (!layout.contains_symbol(symbol->name))
        throw EncodingError("undefined branch target '" + symbol->name + "'");

    const std::int64_t pc =
        section.virtual_address.to_integer() +
        static_cast<std::int64_t>(instruction_offset);

    const std::int64_t target =
        layout.symbol_address(symbol->name).to_integer();

    InstructionIR resolved = instruction;
    resolved.operands[0] = ImmediateOperand{target - pc};

    return resolved;
}

void populate_sections(const Layout& layout, TrnImage& image) {
    for (const auto& section : layout.sections()) {
        image.sections().push_back(
            TrnFormat::SectionEntry{
                format_section_type(section.type),
                format_section_flags(section.type),
                0,
                0,
                section.virtual_address.to_integer(),
                static_cast<std::int64_t>(section.alignment),
                0,
                0
            }
        );
    }
}

void populate_symbols(
    const SymbolTable& symbols,
    const Layout& layout,
    TrnImage& image
) {
    std::vector<const Symbol*> ordered;
    ordered.reserve(symbols.symbols().size());

    for (const auto& [name, symbol] : symbols.symbols())
        ordered.push_back(&symbol);

    std::sort(
        ordered.begin(),
        ordered.end(),
        [](const Symbol* lhs, const Symbol* rhs) {
            return lhs->name < rhs->name;
        }
    );

    for (std::size_t identifier = 0; identifier < ordered.size(); ++identifier) {
        const Symbol& symbol = *ordered[identifier];

        const auto section = section_index(layout, symbol.section);
        const bool is_entry = symbol.name == "_start";

        const auto type =
            is_entry
                ? TrnFormat::SymbolType::Entry
                : symbol.binding == SymbolBinding::Global
                    ? TrnFormat::SymbolType::Global
                    : TrnFormat::SymbolType::Local;

        const auto visibility =
            symbol.binding == SymbolBinding::Global || is_entry
                ? TrnFormat::SymbolVisibility::Global
                : TrnFormat::SymbolVisibility::Local;

        image.symbols().push_back(
            TrnFormat::SymbolEntry{
                static_cast<std::int64_t>(identifier),
                type,
                static_cast<std::int64_t>(section),
                static_cast<std::int64_t>(symbol.offset),
                visibility,
                0
            }
        );

        if (is_entry)
            image.header().start_symbol = static_cast<std::int64_t>(identifier);
    }
}

void populate_header(
    const Layout& layout,
    const SymbolTable& symbols,
    TrnImage& image,
    ternary::Word virtual_base
) {
    auto& header = image.header();

    header.architecture_id = TrnFormat::ArchitectureId;
    header.isa_version = TrnFormat::IsaVersion;
    header.format_version = TrnFormat::FormatVersion;
    header.flags = 0;
    header.memory_base = virtual_base.to_integer();

    if (!layout.has_entry_point())
        throw LayoutError("executable has no entry point; define '_start'");

    header.entry_point = layout.entry_point().to_integer();

    if (!symbols.contains("_start"))
        throw LayoutError("executable has no '_start' symbol");

    for (const auto& section : layout.sections()) {
        const auto address = section.virtual_address.to_integer();

        switch (section.type) {
            case SectionType::Text:
                header.text_address = address;
                break;

            case SectionType::Data:
                header.data_address = address;
                break;

            case SectionType::Rodata:
                header.rodata_address = address;
                break;

            case SectionType::Bss:
                header.bss_address = address;
                break;
        }

        header.memory_size += static_cast<std::int64_t>(section.memory_size);
    }
}

}

TrnImage Assembler::assemble(
    std::string_view source,
    ternary::Word virtual_base
) const {
    if (source.empty())
        throw std::runtime_error("cannot assemble empty source");

    const auto tokens = Lexer(source).tokenize();
    const auto program = Parser(tokens).parse();

    SemanticAnalyzer{}.analyze(program);

    SymbolTable symbol_table;
    symbol_table.build(program);

    SymbolResolver resolver;
    static_cast<void>(resolver.resolve(program, symbol_table));

    Layout layout;
    layout.build(program, symbol_table, virtual_base);

    TrnImage image;

    populate_header(layout, symbol_table, image, virtual_base);
    populate_sections(layout, image);
    populate_symbols(symbol_table, layout, image);

    Encoder encoder;

    std::string current_section;
    std::size_t current_offset = 0;

    for (const auto& statement : program.statements) {
        if (const auto* section = std::get_if<SectionIR>(&statement)) {
            current_section = section->name;
            current_offset = 0;
            continue;
        }

        const auto* instruction = std::get_if<InstructionIR>(&statement);

        if (instruction == nullptr)
            continue;

        const auto* section = layout.section(current_section);

        if (section == nullptr)
            throw LayoutError(
                "instruction encountered outside a valid section"
            );

        if (section->type != SectionType::Text)
            throw LayoutError(
                "instructions are only permitted in .text"
            );

        const auto resolved =
            resolve_branch(
                *instruction,
                current_offset,
                *section,
                layout
            );

        const auto encoded = encoder.encode(resolved);

        image.text().push_back(
            serialize_word(encoded.word())
        );

        ++current_offset;
    }

    for (const auto& section : layout.sections()) {
        if (section.type == SectionType::Bss)
            image.set_bss_size(
                static_cast<std::int64_t>(section.memory_size)
            );
    }

    image.layout();

    return image;
}

void Assembler::assemble_to_file(
    std::string_view source,
    std::string_view output_path,
    ternary::Word virtual_base
) const {
    auto image = assemble(source, virtual_base);
    TrnWriter{}.write(image, output_path);
}

}

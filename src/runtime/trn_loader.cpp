#include "runtime/trn_loader.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

#include "assembler/trn_format.hpp"
#include "ternary/trit.hpp"
#include "ternary/word.hpp"

namespace ternary_machine::runtime {

namespace {

using EncodedWord = ternary_machine::assembler::TrnFormat::Word;
using Format = ternary_machine::assembler::TrnFormat;

[[nodiscard]] std::int64_t decode_word(const EncodedWord& encoded) {
    std::int64_t value = 0;

    for (const char trit : encoded) {
        value *= 3;

        switch (trit) {
            case 'n':
                value -= 1;
                break;

            case '0':
                break;

            case '1':
                value += 1;
                break;

            default:
                throw TrnLoaderError("invalid trit in .trn Word");
        }
    }

    return value;
}

[[nodiscard]] ternary::Word decode_runtime_word(const EncodedWord& encoded) {
    ternary::Word result;

    for (std::size_t i = 0; i < encoded.size(); ++i) {
        switch (encoded[i]) {
            case 'n':
                result.set_trit(i, ternary::Trit::Neg);
                break;

            case '0':
                result.set_trit(i, ternary::Trit::Zero);
                break;

            case '1':
                result.set_trit(i, ternary::Trit::Pos);
                break;

            default:
                throw TrnLoaderError("invalid trit in .trn payload");
        }
    }

    return result;
}

[[nodiscard]] std::vector<EncodedWord> read_words(std::string_view path) {
    if (path.empty())
        throw TrnLoaderError("input path cannot be empty");

    std::ifstream input(std::string(path), std::ios::binary | std::ios::ate);

    if (!input)
        throw TrnLoaderError("failed to open .trn file");

    const auto end = input.tellg();

    if (end < 0)
        throw TrnLoaderError("failed to determine .trn file size");

    const auto byte_count = static_cast<std::uintmax_t>(end);

    if (byte_count % Format::WordTrits != 0)
        throw TrnLoaderError("file size is not an integral number of TVM Words");

    const auto word_count = byte_count / Format::WordTrits;

    if (word_count > std::numeric_limits<std::size_t>::max())
        throw TrnLoaderError(".trn file is too large");

    input.seekg(0, std::ios::beg);

    std::vector<EncodedWord> words(static_cast<std::size_t>(word_count));

    for (auto& word : words) {
        input.read(word.data(), static_cast<std::streamsize>(word.size()));

        if (!input)
            throw TrnLoaderError("failed while reading .trn file");
    }

    return words;
}

[[nodiscard]] std::size_t checked_index(std::int64_t value, std::size_t limit, const char* field) {
    if (value < 0)
        throw TrnLoaderError(std::string(field) + " cannot be negative");

    const auto index = static_cast<std::uintmax_t>(value);

    if (index >= limit)
        throw TrnLoaderError(std::string(field) + " is outside the .trn image");

    return static_cast<std::size_t>(index);
}

[[nodiscard]] std::size_t checked_count(std::int64_t value, const char* field) {
    if (value < 0)
        throw TrnLoaderError(std::string(field) + " cannot be negative");

    if (static_cast<std::uintmax_t>(value) > std::numeric_limits<std::size_t>::max())
        throw TrnLoaderError(std::string(field) + " is too large");

    return static_cast<std::size_t>(value);
}

[[nodiscard]] std::size_t checked_range(
    std::int64_t offset,
    std::int64_t size,
    std::size_t file_words,
    const char* field
) {
    if (offset < 0 || size < 0)
        throw TrnLoaderError(std::string(field) + " contains a negative range");

    const auto begin = static_cast<std::uintmax_t>(offset);
    const auto length = static_cast<std::uintmax_t>(size);
    const auto end = begin + length;

    if (end < begin || end > file_words)
        throw TrnLoaderError(std::string(field) + " extends beyond the .trn file");

    return static_cast<std::size_t>(begin);
}

[[nodiscard]] LoadedImage::SectionType convert_section_type(Format::SectionType type) {
    switch (type) {
        case Format::SectionType::Text:
            return LoadedImage::SectionType::Text;

        case Format::SectionType::Data:
            return LoadedImage::SectionType::Data;

        case Format::SectionType::Rodata:
            return LoadedImage::SectionType::Rodata;

        case Format::SectionType::Bss:
            return LoadedImage::SectionType::Bss;
    }

    throw TrnLoaderError("invalid section type");
}

[[nodiscard]] LoadedImage::SectionType decode_section_type(std::int64_t value) {
    switch (value) {
        case static_cast<std::int64_t>(Format::SectionType::Text):
            return LoadedImage::SectionType::Text;

        case static_cast<std::int64_t>(Format::SectionType::Data):
            return LoadedImage::SectionType::Data;

        case static_cast<std::int64_t>(Format::SectionType::Rodata):
            return LoadedImage::SectionType::Rodata;

        case static_cast<std::int64_t>(Format::SectionType::Bss):
            return LoadedImage::SectionType::Bss;
    }

    throw TrnLoaderError("invalid section type in .trn image");
}

[[nodiscard]] std::int8_t decode_section_flags(std::int64_t value) {
    constexpr std::int64_t valid =
        static_cast<std::int64_t>(Format::SectionFlag::Read) |
        static_cast<std::int64_t>(Format::SectionFlag::Write) |
        static_cast<std::int64_t>(Format::SectionFlag::Execute);

    if (value < 0 || (value & ~valid) != 0)
        throw TrnLoaderError("invalid section flags in .trn image");

    return static_cast<std::int8_t>(value);
}

[[nodiscard]] std::int8_t decode_symbol_type(std::int64_t value) {
    switch (value) {
        case static_cast<std::int64_t>(Format::SymbolType::Local):
        case static_cast<std::int64_t>(Format::SymbolType::Global):
        case static_cast<std::int64_t>(Format::SymbolType::Entry):
            return static_cast<std::int8_t>(value);
    }

    throw TrnLoaderError("invalid symbol type in .trn image");
}

[[nodiscard]] std::int64_t checked_add(
    std::int64_t lhs,
    std::int64_t rhs,
    const char* field
) {
    if ((rhs > 0 && lhs > std::numeric_limits<std::int64_t>::max() - rhs) ||
        (rhs < 0 && lhs < std::numeric_limits<std::int64_t>::min() - rhs))
        throw TrnLoaderError(std::string(field) + " overflows");

    return lhs + rhs;
}

}

LoadedImage TrnLoader::load(std::string_view path) const {
    const auto words = read_words(path);

    if (words.size() < Format::HeaderWords)
        throw TrnLoaderError("truncated .trn header");

    Format::Header header;

    header.architecture_id = decode_word(words[0]);
    header.isa_version = decode_word(words[1]);
    header.format_version = decode_word(words[2]);
    header.flags = decode_word(words[3]);
    header.entry_point = decode_word(words[4]);
    header.start_symbol = decode_word(words[5]);
    header.section_table = decode_word(words[6]);
    header.section_count = decode_word(words[7]);
    header.symbol_table = decode_word(words[8]);
    header.symbol_count = decode_word(words[9]);
    header.memory_base = decode_word(words[10]);
    header.memory_size = decode_word(words[11]);
    header.text_address = decode_word(words[12]);
    header.data_address = decode_word(words[13]);
    header.rodata_address = decode_word(words[14]);
    header.bss_address = decode_word(words[15]);

    if (header.architecture_id != Format::ArchitectureId)
        throw TrnLoaderError("unsupported TVM architecture identifier");

    if (header.isa_version != Format::IsaVersion)
        throw TrnLoaderError("unsupported TVM ISA version");

    if (header.format_version != Format::FormatVersion)
        throw TrnLoaderError("unsupported .trn format version");

    if (header.section_table != static_cast<std::int64_t>(Format::HeaderWords))
        throw TrnLoaderError("invalid section-table location");

    if (header.section_count < 0)
        throw TrnLoaderError("negative section count");

    if (header.symbol_count < 0)
        throw TrnLoaderError("negative symbol count");

    const auto section_count = checked_count(header.section_count, "section count");
    const auto symbol_count = checked_count(header.symbol_count, "symbol count");

    const auto section_table_end =
        checked_add(
            header.section_table,
            static_cast<std::int64_t>(section_count * Format::SectionEntryWords),
            "section table"
        );

    if (header.symbol_table != section_table_end)
        throw TrnLoaderError("invalid symbol-table location");

    const auto symbol_table_end =
        checked_add(
            header.symbol_table,
            static_cast<std::int64_t>(symbol_count * Format::SymbolEntryWords),
            "symbol table"
        );

    if (symbol_table_end < 0 ||
        static_cast<std::uintmax_t>(symbol_table_end) > words.size())
        throw TrnLoaderError("metadata extends beyond the .trn file");

    LoadedImage image;

    image.set_architecture_id(header.architecture_id);
    image.set_isa_version(header.isa_version);
    image.set_format_version(header.format_version);
    image.set_flags(header.flags);
    image.set_entry_point(header.entry_point);
    image.set_start_symbol(header.start_symbol);
    image.set_memory_base(header.memory_base);
    image.set_memory_size(header.memory_size);

    std::vector<std::pair<std::size_t, std::size_t>> payload_ranges;
    payload_ranges.reserve(section_count);

    for (std::size_t i = 0; i < section_count; ++i) {
        const auto base =
            static_cast<std::size_t>(header.section_table) +
            i * Format::SectionEntryWords;

        const auto type = decode_section_type(decode_word(words[base]));
        const auto flags = decode_section_flags(decode_word(words[base + 1]));
        const auto file_offset = decode_word(words[base + 2]);
        const auto size = decode_word(words[base + 3]);
        const auto virtual_address = decode_word(words[base + 4]);
        const auto alignment = decode_word(words[base + 5]);
        const auto reserved0 = decode_word(words[base + 6]);
        const auto reserved1 = decode_word(words[base + 7]);

        if (reserved0 != 0 || reserved1 != 0)
            throw TrnLoaderError("non-zero reserved section field");

        if (size < 0)
            throw TrnLoaderError("negative section size");

        if (alignment <= 0)
            throw TrnLoaderError("invalid section alignment");

        LoadedImage::Section section;
        section.type = type;
        section.flags = flags;
        section.virtual_address = virtual_address;
        section.memory_size = size;
        section.alignment = alignment;

        if (type == LoadedImage::SectionType::Bss) {
            if (file_offset != 0)
                throw TrnLoaderError("BSS section has physical file payload");

            if (size != 0)
                section.words.clear();
        } else {
            const auto payload_begin =
                checked_range(file_offset, size, words.size(), "section payload");

            if (file_offset < symbol_table_end)
                throw TrnLoaderError("section payload overlaps executable metadata");

            for (std::size_t word = 0; word < static_cast<std::size_t>(size); ++word)
                section.words.push_back(
                    decode_runtime_word(words[payload_begin + word])
                );

            if (size != 0)
                payload_ranges.emplace_back(
                    payload_begin,
                    payload_begin + static_cast<std::size_t>(size)
                );
        }

        image.sections().push_back(std::move(section));
    }

    for (std::size_t i = 0; i < payload_ranges.size(); ++i) {
        for (std::size_t j = i + 1; j < payload_ranges.size(); ++j) {
            const auto [a_begin, a_end] = payload_ranges[i];
            const auto [b_begin, b_end] = payload_ranges[j];

            if (a_begin < b_end && b_begin < a_end)
                throw TrnLoaderError("overlapping section payloads");
        }
    }

    for (std::size_t i = 0; i < symbol_count; ++i) {
        const auto base =
            static_cast<std::size_t>(header.symbol_table) +
            i * Format::SymbolEntryWords;

        const auto identifier = decode_word(words[base]);
        const auto type = decode_symbol_type(decode_word(words[base + 1]));
        const auto section = decode_word(words[base + 2]);
        const auto offset = decode_word(words[base + 3]);
        const auto visibility = decode_word(words[base + 4]);
        const auto reserved = decode_word(words[base + 5]);

        if (identifier != static_cast<std::int64_t>(i))
            throw TrnLoaderError("symbol identifiers are not sequential");

        if (section < 0 ||
            static_cast<std::uintmax_t>(section) >= image.sections().size())
            throw TrnLoaderError("symbol references an invalid section");

        if (offset < 0)
            throw TrnLoaderError("symbol offset cannot be negative");

        if (visibility != static_cast<std::int64_t>(Format::SymbolVisibility::Local) &&
            visibility != static_cast<std::int64_t>(Format::SymbolVisibility::Global))
            throw TrnLoaderError("invalid symbol visibility");

        if (reserved != 0)
            throw TrnLoaderError("non-zero reserved symbol field");

        const auto& target_section = image.sections()[static_cast<std::size_t>(section)];

        if (offset >= target_section.memory_size)
            throw TrnLoaderError("symbol offset lies outside its section");

        image.symbols().push_back(
            LoadedImage::Symbol{
                identifier,
                type,
                section,
                checked_add(target_section.virtual_address, offset, "symbol address")
            }
        );
    }

    if (header.start_symbol < -1)
        throw TrnLoaderError("invalid start-symbol identifier");

    if (header.start_symbol >= 0) {
        if (static_cast<std::uintmax_t>(header.start_symbol) >= image.symbols().size())
            throw TrnLoaderError("start symbol lies outside the symbol table");

        if (image.symbols()[static_cast<std::size_t>(header.start_symbol)].type !=
            static_cast<std::int8_t>(Format::SymbolType::Entry))
            throw TrnLoaderError("start symbol is not an ENTRY symbol");
    }

    if (header.memory_size < 0)
        throw TrnLoaderError("negative memory size");

    for (const auto& section : image.sections()) {
        if (section.virtual_address < header.memory_base)
            throw TrnLoaderError("section lies below memory base");

        const auto end =
            checked_add(
                section.virtual_address,
                section.memory_size,
                "section memory range"
            );

        const auto memory_end =
            checked_add(header.memory_base, header.memory_size, "memory range");

        if (end > memory_end)
            throw TrnLoaderError("section lies outside declared memory");
    }

    const auto* text = image.text();

    if (text == nullptr)
        throw TrnLoaderError("executable contains no .text section");

    if (header.entry_point < text->virtual_address ||
        header.entry_point >=
            checked_add(text->virtual_address, text->memory_size, "entry-point range"))
        throw TrnLoaderError("entry point lies outside .text");

    return image;
}

}

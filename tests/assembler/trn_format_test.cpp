#include <cassert>
#include <string>

#include "assembler/trn_format.hpp"

using ternary_machine::assembler::TrnFormat;
using ternary_machine::assembler::TrnFormatError;

int main() {
    {
        const auto word = TrnFormat::encode_word(0);

        assert(TrnFormat::decode_word(word) == 0);
        assert(TrnFormat::word_string(word).size() == 27);
        assert(TrnFormat::word_string(word) == "000000000000000000000000000");
    }

    {
        const auto word = TrnFormat::encode_word(1);

        assert(TrnFormat::decode_word(word) == 1);
        assert(TrnFormat::word_string(word).size() == 27);
    }

    {
        const auto word = TrnFormat::encode_word(-1);

        assert(TrnFormat::decode_word(word) == -1);
        assert(TrnFormat::word_string(word).size() == 27);
    }

    {
        const auto word = TrnFormat::encode_word(42);

        assert(TrnFormat::decode_word(word) == 42);

        for (const char trit : word)
            assert(trit == 'n' || trit == '0' || trit == '1');
    }

    {
        const auto word = TrnFormat::encode_word(-5678);

        assert(TrnFormat::decode_word(word) == -5678);

        for (const char trit : word)
            assert(trit == 'n' || trit == '0' || trit == '1');
    }

    {
        const auto minimum = TrnFormat::encode_word(TrnFormat::minimum_word_value());
        const auto maximum = TrnFormat::encode_word(TrnFormat::maximum_word_value());

        assert(TrnFormat::decode_word(minimum) == TrnFormat::minimum_word_value());
        assert(TrnFormat::decode_word(maximum) == TrnFormat::maximum_word_value());
    }

    {
        bool rejected = false;

        try {
            static_cast<void>(TrnFormat::encode_word(TrnFormat::maximum_word_value() + 1));
        } catch (const TrnFormatError&) {
            rejected = true;
        }

        assert(rejected);
    }

    {
        bool rejected = false;

        try {
            static_cast<void>(TrnFormat::encode_word(TrnFormat::minimum_word_value() - 1));
        } catch (const TrnFormatError&) {
            rejected = true;
        }

        assert(rejected);
    }

    {
        TrnFormat::Word invalid{};
        invalid.fill('0');
        invalid[13] = 'x';

        bool rejected = false;

        try {
            static_cast<void>(TrnFormat::decode_word(invalid));
        } catch (const TrnFormatError&) {
            rejected = true;
        }

        assert(rejected);
    }

    {
        TrnFormat::Header header;

        header.architecture_id = TrnFormat::ArchitectureId;
        header.isa_version = TrnFormat::IsaVersion;
        header.format_version = TrnFormat::FormatVersion;
        header.flags = 7;
        header.entry_point = 42;
        header.start_symbol = 1;
        header.section_table = 16;
        header.section_count = 4;
        header.symbol_table = 48;
        header.symbol_count = 2;
        header.memory_base = 0;
        header.memory_size = 1024;
        header.text_address = 0;
        header.data_address = 128;
        header.rodata_address = 256;
        header.bss_address = 384;

        const auto encoded = TrnFormat::encode_header(header);

        assert(encoded.size() == TrnFormat::HeaderWords);
        assert(TrnFormat::decode_word(encoded[0]) == TrnFormat::ArchitectureId);
        assert(TrnFormat::decode_word(encoded[1]) == TrnFormat::IsaVersion);
        assert(TrnFormat::decode_word(encoded[2]) == TrnFormat::FormatVersion);
        assert(TrnFormat::decode_word(encoded[4]) == 42);
        assert(TrnFormat::decode_word(encoded[5]) == 1);
        assert(TrnFormat::decode_word(encoded[7]) == 4);
        assert(TrnFormat::decode_word(encoded[9]) == 2);
        assert(TrnFormat::decode_word(encoded[13]) == 128);
        assert(TrnFormat::decode_word(encoded[15]) == 384);
    }

    {
        TrnFormat::SectionEntry section;

        section.type = TrnFormat::SectionType::Text;
        section.flags = static_cast<std::int64_t>(TrnFormat::SectionFlag::Read) | static_cast<std::int64_t>(TrnFormat::SectionFlag::Execute);
        section.file_offset = 64;
        section.size = 12;
        section.virtual_address = 0;
        section.alignment = 1;

        const auto encoded = TrnFormat::encode_section(section);

        assert(encoded.size() == TrnFormat::SectionEntryWords);
        assert(TrnFormat::decode_word(encoded[0]) == 0);
        assert(TrnFormat::decode_word(encoded[1]) == 5);
        assert(TrnFormat::decode_word(encoded[2]) == 64);
        assert(TrnFormat::decode_word(encoded[3]) == 12);
        assert(TrnFormat::decode_word(encoded[4]) == 0);
    }

    {
        TrnFormat::SymbolEntry symbol;

        symbol.identifier = 1;
        symbol.type = TrnFormat::SymbolType::Entry;
        symbol.section = 0;
        symbol.offset = 12;
        symbol.visibility = TrnFormat::SymbolVisibility::Global;

        const auto encoded = TrnFormat::encode_symbol(symbol);

        assert(encoded.size() == TrnFormat::SymbolEntryWords);
        assert(TrnFormat::decode_word(encoded[0]) == 1);
        assert(TrnFormat::decode_word(encoded[1]) == 2);
        assert(TrnFormat::decode_word(encoded[2]) == 0);
        assert(TrnFormat::decode_word(encoded[3]) == 12);
        assert(TrnFormat::decode_word(encoded[4]) == 1);
    }

    return 0;
}

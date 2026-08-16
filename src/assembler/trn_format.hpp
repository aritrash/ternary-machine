#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ternary_machine::assembler {

class TrnFormatError final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class TrnFormat final {
public:
    static constexpr std::size_t WordTrits = 27;

    using Word = std::array<char, WordTrits>;

    enum class SectionType : std::int64_t {
        Text = 0,
        Data = 1,
        Rodata = 2,
        Bss = 3
    };

    enum class SectionFlag : std::int64_t {
        None = 0,
        Read = 1,
        Write = 2,
        Execute = 4
    };

    enum class SymbolType : std::int64_t {
        Local = 0,
        Global = 1,
        Entry = 2
    };

    enum class SymbolVisibility : std::int64_t {
        Local = 0,
        Global = 1
    };

    static constexpr std::int64_t ArchitectureId = 1;
    static constexpr std::int64_t IsaVersion = 1;
    static constexpr std::int64_t FormatVersion = 1;

    static constexpr std::size_t HeaderWords = 16;
    static constexpr std::size_t SectionEntryWords = 8;
    static constexpr std::size_t SymbolEntryWords = 6;

    struct Header final {
        std::int64_t architecture_id = ArchitectureId;
        std::int64_t isa_version = IsaVersion;
        std::int64_t format_version = FormatVersion;
        std::int64_t flags = 0;
        std::int64_t entry_point = 0;
        std::int64_t start_symbol = 0;
        std::int64_t section_table = HeaderWords;
        std::int64_t section_count = 0;
        std::int64_t symbol_table = 0;
        std::int64_t symbol_count = 0;
        std::int64_t memory_base = 0;
        std::int64_t memory_size = 0;
        std::int64_t text_address = 0;
        std::int64_t data_address = 0;
        std::int64_t rodata_address = 0;
        std::int64_t bss_address = 0;
    };

    struct SectionEntry final {
        SectionType type = SectionType::Text;
        std::int64_t flags = 0;
        std::int64_t file_offset = 0;
        std::int64_t size = 0;
        std::int64_t virtual_address = 0;
        std::int64_t alignment = 1;
        std::int64_t reserved0 = 0;
        std::int64_t reserved1 = 0;
    };

    struct SymbolEntry final {
        std::int64_t identifier = 0;
        SymbolType type = SymbolType::Local;
        std::int64_t section = 0;
        std::int64_t offset = 0;
        SymbolVisibility visibility = SymbolVisibility::Local;
        std::int64_t reserved = 0;
    };

    [[nodiscard]] static constexpr std::size_t header_words() noexcept {
        return HeaderWords;
    }

    [[nodiscard]] static constexpr std::size_t section_entry_words() noexcept {
        return SectionEntryWords;
    }

    [[nodiscard]] static constexpr std::size_t symbol_entry_words() noexcept {
        return SymbolEntryWords;
    }

    [[nodiscard]] static Word encode_word(std::int64_t value) {
        if (value < minimum_word_value() || value > maximum_word_value())
            throw TrnFormatError("value does not fit in a 27-trit Word");

        Word result{};

        std::int64_t remaining = value;

        for (std::size_t i = 0; i < WordTrits; ++i) {
            const auto remainder = balanced_remainder(remaining);
            result[WordTrits - 1 - i] = trit_character(remainder);
            remaining = (remaining - remainder) / 3;
        }

        return result;
    }

    [[nodiscard]] static std::int64_t decode_word(const Word& word) {
        std::int64_t value = 0;

        for (const char trit : word) {
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
                    throw TrnFormatError("invalid trit in encoded Word");
            }
        }

        return value;
    }

    [[nodiscard]] static std::string word_string(const Word& word) {
        return std::string(word.data(), word.size());
    }

    [[nodiscard]] static constexpr std::int64_t minimum_word_value() noexcept {
        return -((power_of_three(WordTrits) - 1) / 2);
    }

    [[nodiscard]] static constexpr std::int64_t maximum_word_value() noexcept {
        return (power_of_three(WordTrits) - 1) / 2;
    }

    [[nodiscard]] static std::array<Word, HeaderWords> encode_header(const Header& header) {
        return {
            encode_word(header.architecture_id),
            encode_word(header.isa_version),
            encode_word(header.format_version),
            encode_word(header.flags),
            encode_word(header.entry_point),
            encode_word(header.start_symbol),
            encode_word(header.section_table),
            encode_word(header.section_count),
            encode_word(header.symbol_table),
            encode_word(header.symbol_count),
            encode_word(header.memory_base),
            encode_word(header.memory_size),
            encode_word(header.text_address),
            encode_word(header.data_address),
            encode_word(header.rodata_address),
            encode_word(header.bss_address)
        };
    }

    [[nodiscard]] static std::array<Word, SectionEntryWords> encode_section(const SectionEntry& section) {
        return {
            encode_word(static_cast<std::int64_t>(section.type)),
            encode_word(section.flags),
            encode_word(section.file_offset),
            encode_word(section.size),
            encode_word(section.virtual_address),
            encode_word(section.alignment),
            encode_word(section.reserved0),
            encode_word(section.reserved1)
        };
    }

    [[nodiscard]] static std::array<Word, SymbolEntryWords> encode_symbol(const SymbolEntry& symbol) {
        return {
            encode_word(symbol.identifier),
            encode_word(static_cast<std::int64_t>(symbol.type)),
            encode_word(symbol.section),
            encode_word(symbol.offset),
            encode_word(static_cast<std::int64_t>(symbol.visibility)),
            encode_word(symbol.reserved)
        };
    }

private:
    [[nodiscard]] static constexpr std::int64_t power_of_three(std::size_t exponent) noexcept {
        std::int64_t value = 1;

        for (std::size_t i = 0; i < exponent; ++i)
            value *= 3;

        return value;
    }

    [[nodiscard]] static constexpr std::int64_t balanced_remainder(std::int64_t value) noexcept {
        const auto remainder = value % 3;

        if (remainder == 2)
            return -1;

        if (remainder == -2)
            return 1;

        return remainder;
    }

    [[nodiscard]] static constexpr char trit_character(std::int64_t trit) noexcept {
        return trit < 0 ? 'n' : trit == 0 ? '0' : '1';
    }
};

} 

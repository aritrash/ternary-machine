#pragma once

#include <cstdint>
#include <vector>

#include "trn_format.hpp"

namespace ternary_machine::assembler {

class TrnImage final {
public:
    using Word = TrnFormat::Word;

    [[nodiscard]] TrnFormat::Header& header() noexcept {
        return header_;
    }

    [[nodiscard]] const TrnFormat::Header& header() const noexcept {
        return header_;
    }

    [[nodiscard]] std::vector<TrnFormat::SectionEntry>& sections() noexcept {
        return sections_;
    }

    [[nodiscard]] const std::vector<TrnFormat::SectionEntry>& sections() const noexcept {
        return sections_;
    }

    [[nodiscard]] std::vector<TrnFormat::SymbolEntry>& symbols() noexcept {
        return symbols_;
    }

    [[nodiscard]] const std::vector<TrnFormat::SymbolEntry>& symbols() const noexcept {
        return symbols_;
    }

    [[nodiscard]] std::vector<Word>& text() noexcept {
        return text_;
    }

    [[nodiscard]] const std::vector<Word>& text() const noexcept {
        return text_;
    }

    [[nodiscard]] std::vector<Word>& data() noexcept {
        return data_;
    }

    [[nodiscard]] const std::vector<Word>& data() const noexcept {
        return data_;
    }

    [[nodiscard]] std::vector<Word>& rodata() noexcept {
        return rodata_;
    }

    [[nodiscard]] const std::vector<Word>& rodata() const noexcept {
        return rodata_;
    }

    [[nodiscard]] std::int64_t bss_size() const noexcept {
        return bss_size_;
    }

    void set_bss_size(std::int64_t size) {
        if (size < 0)
            throw TrnFormatError("BSS size cannot be negative");

        bss_size_ = size;
        laid_out_ = false;
    }

    void layout() {
        header_.section_count = static_cast<std::int64_t>(sections_.size());
        header_.symbol_count = static_cast<std::int64_t>(symbols_.size());
        header_.section_table = static_cast<std::int64_t>(TrnFormat::HeaderWords);

        header_.symbol_table = header_.section_table +
            static_cast<std::int64_t>(sections_.size() * TrnFormat::SectionEntryWords);

        std::int64_t cursor = header_.symbol_table +
            static_cast<std::int64_t>(symbols_.size() * TrnFormat::SymbolEntryWords);

        for (auto& section : sections_) {
            switch (section.type) {
                case TrnFormat::SectionType::Text:
                    section.file_offset = cursor;
                    section.size = static_cast<std::int64_t>(text_.size());
                    cursor += section.size;
                    break;

                case TrnFormat::SectionType::Data:
                    section.file_offset = cursor;
                    section.size = static_cast<std::int64_t>(data_.size());
                    cursor += section.size;
                    break;

                case TrnFormat::SectionType::Rodata:
                    section.file_offset = cursor;
                    section.size = static_cast<std::int64_t>(rodata_.size());
                    cursor += section.size;
                    break;

                case TrnFormat::SectionType::Bss:
                    section.file_offset = 0;
                    section.size = bss_size_;
                    break;
            }
        }

        laid_out_ = true;
    }

    [[nodiscard]] bool is_laid_out() const noexcept {
        return laid_out_;
    }

    [[nodiscard]] std::vector<Word> serialize() const {
        if (!laid_out_)
            throw TrnFormatError("cannot serialize an image before layout");

        std::vector<Word> image;

        const auto total_words = payload_end();
        image.reserve(static_cast<std::size_t>(total_words));

        const auto encoded_header = TrnFormat::encode_header(header_);
        image.insert(image.end(), encoded_header.begin(), encoded_header.end());

        for (const auto& section : sections_) {
            const auto encoded = TrnFormat::encode_section(section);
            image.insert(image.end(), encoded.begin(), encoded.end());
        }

        for (const auto& symbol : symbols_) {
            const auto encoded = TrnFormat::encode_symbol(symbol);
            image.insert(image.end(), encoded.begin(), encoded.end());
        }

        append_payload(image, text_);
        append_payload(image, data_);
        append_payload(image, rodata_);

        return image;
    }

private:
    [[nodiscard]] std::int64_t payload_end() const noexcept {
        std::int64_t end = header_.symbol_table +
            static_cast<std::int64_t>(symbols_.size() * TrnFormat::SymbolEntryWords);

        for (const auto& section : sections_) {
            if (section.type == TrnFormat::SectionType::Bss)
                continue;

            const auto section_end = section.file_offset + section.size;

            if (section_end > end)
                end = section_end;
        }

        return end;
    }

    static void append_payload(std::vector<Word>& image, const std::vector<Word>& payload) {
        image.insert(image.end(), payload.begin(), payload.end());
    }

    TrnFormat::Header header_{};
    std::vector<TrnFormat::SectionEntry> sections_;
    std::vector<TrnFormat::SymbolEntry> symbols_;
    std::vector<Word> text_;
    std::vector<Word> data_;
    std::vector<Word> rodata_;
    std::int64_t bss_size_ = 0;
    bool laid_out_ = false;
};

}

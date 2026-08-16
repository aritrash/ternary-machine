#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include "assembler/trn_format.hpp"
#include "assembler/trn_image.hpp"
#include "assembler/trn_writer.hpp"

using ternary_machine::assembler::TrnFormat;
using ternary_machine::assembler::TrnImage;
using ternary_machine::assembler::TrnWriter;
using ternary_machine::assembler::TrnWriterError;

static std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    assert(input);
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

static bool rejects_write(const TrnImage& image, const std::filesystem::path& path) {
    try {
        TrnWriter{}.write(image, path.string());
    } catch (const TrnWriterError&) {
        return true;
    }

    return false;
}

static void assert_ternary(const std::string& contents) {
    for (const char character : contents)
        assert(character == 'n' || character == '0' || character == '1');
}

int main() {
    const auto path = std::filesystem::temp_directory_path() / "ternary_machine_trn_writer_test.trn";

    {
        TrnImage image;
        image.layout();

        TrnWriter{}.write(image, path.string());

        const auto contents = read_file(path);

        assert(contents.size() == TrnFormat::HeaderWords * TrnFormat::WordTrits);
        assert(contents.size() % TrnFormat::WordTrits == 0);
        assert_ternary(contents);
    }

    {
        TrnImage image;

        image.text().push_back(TrnFormat::encode_word(1));
        image.text().push_back(TrnFormat::encode_word(-1));
        image.text().push_back(TrnFormat::encode_word(42));

        TrnFormat::SectionEntry text;
        text.type = TrnFormat::SectionType::Text;
        text.flags = static_cast<std::int64_t>(TrnFormat::SectionFlag::Read) |
                     static_cast<std::int64_t>(TrnFormat::SectionFlag::Execute);

        image.sections().push_back(text);
        image.layout();

        TrnWriter{}.write(image, path.string());

        const auto contents = read_file(path);
        const auto expected_words = TrnFormat::HeaderWords + TrnFormat::SectionEntryWords + 3;

        assert(contents.size() == expected_words * TrnFormat::WordTrits);
        assert_ternary(contents);

        const auto payload_offset = (expected_words - 3) * TrnFormat::WordTrits;

        assert(contents.substr(payload_offset, TrnFormat::WordTrits) ==
               TrnFormat::word_string(TrnFormat::encode_word(1)));

        assert(contents.substr(payload_offset + TrnFormat::WordTrits, TrnFormat::WordTrits) ==
               TrnFormat::word_string(TrnFormat::encode_word(-1)));

        assert(contents.substr(payload_offset + 2 * TrnFormat::WordTrits, TrnFormat::WordTrits) ==
               TrnFormat::word_string(TrnFormat::encode_word(42)));
    }

    {
        TrnImage image;

        image.text().push_back(TrnFormat::encode_word(7));
        image.data().push_back(TrnFormat::encode_word(11));
        image.rodata().push_back(TrnFormat::encode_word(-13));
        image.set_bss_size(8);

        TrnFormat::SectionEntry text;
        text.type = TrnFormat::SectionType::Text;

        TrnFormat::SectionEntry data;
        data.type = TrnFormat::SectionType::Data;

        TrnFormat::SectionEntry rodata;
        rodata.type = TrnFormat::SectionType::Rodata;

        TrnFormat::SectionEntry bss;
        bss.type = TrnFormat::SectionType::Bss;

        image.sections().push_back(text);
        image.sections().push_back(data);
        image.sections().push_back(rodata);
        image.sections().push_back(bss);

        image.layout();

        assert(image.sections().size() == 4);
        assert(image.sections()[3].type == TrnFormat::SectionType::Bss);
        assert(image.sections()[3].file_offset == 0);
        assert(image.sections()[3].size == 8);

        TrnWriter{}.write(image, path.string());

        const auto contents = read_file(path);

        const auto expected_words =
            TrnFormat::HeaderWords +
            4 * TrnFormat::SectionEntryWords +
            3;

        assert(contents.size() == expected_words * TrnFormat::WordTrits);
        assert(contents.size() == (TrnFormat::HeaderWords + 4 * TrnFormat::SectionEntryWords + 3) * TrnFormat::WordTrits);

        assert_ternary(contents);

        assert(contents.find(TrnFormat::word_string(TrnFormat::encode_word(7))) != std::string::npos);
        assert(contents.find(TrnFormat::word_string(TrnFormat::encode_word(11))) != std::string::npos);
        assert(contents.find(TrnFormat::word_string(TrnFormat::encode_word(-13))) != std::string::npos);
    }

    {
        TrnImage image;

        assert(rejects_write(image, path));
    }

    {
        TrnImage image;
        image.layout();

        assert(rejects_write(image, ""));
    }

    std::filesystem::remove(path);

    return 0;
}

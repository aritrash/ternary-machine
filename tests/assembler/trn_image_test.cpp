#include <cassert>
#include <vector>

#include "assembler/trn_image.hpp"

using ternary_machine::assembler::TrnFormat;
using ternary_machine::assembler::TrnFormatError;
using ternary_machine::assembler::TrnImage;

int main() {
    {
        TrnImage image;

        assert(!image.is_laid_out());

        bool rejected = false;

        try {
            static_cast<void>(image.serialize());
        } catch (const TrnFormatError&) {
            rejected = true;
        }

        assert(rejected);
    }

    {
        TrnImage image;

        image.header().architecture_id = TrnFormat::ArchitectureId;
        image.header().isa_version = TrnFormat::IsaVersion;
        image.header().format_version = TrnFormat::FormatVersion;
        image.header().entry_point = 0;
        image.header().start_symbol = 1;

        image.sections().push_back({
            TrnFormat::SectionType::Text,
            5,
            0,
            0,
            0,
            1,
            0,
            0
        });

        image.symbols().push_back({
            1,
            TrnFormat::SymbolType::Entry,
            0,
            0,
            TrnFormat::SymbolVisibility::Global,
            0
        });

        image.text().push_back(TrnFormat::encode_word(0));
        image.text().push_back(TrnFormat::encode_word(42));

        image.layout();

        assert(image.is_laid_out());

        assert(image.header().section_count == 1);
        assert(image.header().symbol_count == 1);
        assert(image.header().section_table == 16);
        assert(image.header().symbol_table == 24);

        assert(image.sections()[0].file_offset == 30);
        assert(image.sections()[0].size == 2);

        const auto serialized = image.serialize();

        assert(serialized.size() == 32);

        assert(TrnFormat::decode_word(serialized[0]) == TrnFormat::ArchitectureId);
        assert(TrnFormat::decode_word(serialized[1]) == TrnFormat::IsaVersion);
        assert(TrnFormat::decode_word(serialized[2]) == TrnFormat::FormatVersion);

        assert(TrnFormat::decode_word(serialized[7]) == 1);
        assert(TrnFormat::decode_word(serialized[9]) == 1);

        assert(TrnFormat::decode_word(serialized[16]) == 0);
        assert(TrnFormat::decode_word(serialized[17]) == 5);
        assert(TrnFormat::decode_word(serialized[18]) == 30);
        assert(TrnFormat::decode_word(serialized[19]) == 2);

        assert(TrnFormat::decode_word(serialized[24]) == 1);
        assert(TrnFormat::decode_word(serialized[25]) == 2);

        assert(TrnFormat::decode_word(serialized[30]) == 0);
        assert(TrnFormat::decode_word(serialized[31]) == 42);
    }

    {
        TrnImage image;

        image.sections().push_back({
            TrnFormat::SectionType::Text,
            5,
            0,
            0,
            0,
            1,
            0,
            0
        });

        image.sections().push_back({
            TrnFormat::SectionType::Data,
            3,
            0,
            0,
            128,
            1,
            0,
            0
        });

        image.sections().push_back({
            TrnFormat::SectionType::Rodata,
            1,
            0,
            0,
            256,
            1,
            0,
            0
        });

        image.sections().push_back({
            TrnFormat::SectionType::Bss,
            3,
            0,
            0,
            384,
            1,
            0,
            0
        });

        image.text().push_back(TrnFormat::encode_word(11));
        image.text().push_back(TrnFormat::encode_word(22));

        image.data().push_back(TrnFormat::encode_word(33));

        image.rodata().push_back(TrnFormat::encode_word(44));
        image.rodata().push_back(TrnFormat::encode_word(55));

        image.set_bss_size(8);
        image.layout();

        assert(image.sections()[0].file_offset == 48);
        assert(image.sections()[0].size == 2);

        assert(image.sections()[1].file_offset == 50);
        assert(image.sections()[1].size == 1);

        assert(image.sections()[2].file_offset == 51);
        assert(image.sections()[2].size == 2);

        assert(image.sections()[3].file_offset == 0);
        assert(image.sections()[3].size == 8);

        const auto serialized = image.serialize();

        assert(serialized.size() == 53);

        assert(TrnFormat::decode_word(serialized[48]) == 11);
        assert(TrnFormat::decode_word(serialized[49]) == 22);

        assert(TrnFormat::decode_word(serialized[50]) == 33);

        assert(TrnFormat::decode_word(serialized[51]) == 44);
        assert(TrnFormat::decode_word(serialized[52]) == 55);
    }

    {
        TrnImage image;

        image.sections().push_back({
            TrnFormat::SectionType::Bss,
            3,
            0,
            0,
            1024,
            1,
            0,
            0
        });

        image.set_bss_size(16);
        image.layout();

        assert(image.sections()[0].file_offset == 0);
        assert(image.sections()[0].size == 16);

        const auto serialized = image.serialize();

        assert(serialized.size() == 24);
    }

    {
        TrnImage image;

        bool rejected = false;

        try {
            image.set_bss_size(-1);
        } catch (const TrnFormatError&) {
            rejected = true;
        }

        assert(rejected);
    }

    {
        TrnImage image;

        image.sections().push_back({
            TrnFormat::SectionType::Text,
            5,
            0,
            0,
            0,
            1,
            0,
            0
        });

        image.text().push_back(TrnFormat::encode_word(123));

        image.layout();

        assert(image.is_laid_out());

        image.set_bss_size(4);

        assert(!image.is_laid_out());
    }

    {
        TrnImage image;

        image.sections().push_back({
            TrnFormat::SectionType::Text,
            5,
            0,
            0,
            0,
            1,
            0,
            0
        });

        image.text().push_back(TrnFormat::encode_word(123));

        image.layout();

        const auto first = image.serialize();
        const auto second = image.serialize();

        assert(first == second);
    }

    return 0;
}

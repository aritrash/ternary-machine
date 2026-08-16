#include <cassert>
#include <cstdint>

#include "runtime/loaded_image.hpp"
#include "ternary/word.hpp"

using ternary_machine::runtime::LoadedImage;
using ternary_machine::ternary::Word;

int main() {
    {
        LoadedImage image;

        assert(image.architecture_id() == 0);
        assert(image.isa_version() == 0);
        assert(image.format_version() == 0);
        assert(image.flags() == 0);
        assert(image.entry_point() == 0);
        assert(image.start_symbol() == -1);
        assert(image.memory_base() == 0);
        assert(image.memory_size() == 0);

        assert(image.sections().empty());
        assert(image.symbols().empty());

        assert(image.text() == nullptr);
        assert(image.data() == nullptr);
        assert(image.rodata() == nullptr);
        assert(image.bss() == nullptr);
    }

    {
        LoadedImage image;

        image.set_architecture_id(1);
        image.set_isa_version(1);
        image.set_format_version(1);
        image.set_flags(7);
        image.set_memory_base(100);
        image.set_memory_size(50);
        image.set_entry_point(100);
        image.set_start_symbol(0);

        assert(image.architecture_id() == 1);
        assert(image.isa_version() == 1);
        assert(image.format_version() == 1);
        assert(image.flags() == 7);
        assert(image.memory_base() == 100);
        assert(image.memory_size() == 50);
        assert(image.entry_point() == 100);
        assert(image.start_symbol() == 0);
    }

    {
        LoadedImage image;

        LoadedImage::Section text;
        text.type = LoadedImage::SectionType::Text;
        text.flags =
            static_cast<std::int8_t>(LoadedImage::SectionFlags::Read) |
            static_cast<std::int8_t>(LoadedImage::SectionFlags::Execute);
        text.virtual_address = 100;
        text.memory_size = 2;
        text.alignment = 1;
        text.words.push_back(Word::from_integer(42));
        text.words.push_back(Word::from_integer(-17));

        image.sections().push_back(text);

        assert(image.sections().size() == 1);
        assert(image.has_section(LoadedImage::SectionType::Text));

        const auto* section = image.text();

        assert(section != nullptr);
        assert(section->type == LoadedImage::SectionType::Text);
        assert(section->virtual_address == 100);
        assert(section->memory_size == 2);
        assert(section->alignment == 1);
        assert(section->words.size() == 2);
        assert(section->words[0] == Word::from_integer(42));
        assert(section->words[1] == Word::from_integer(-17));

        assert(section->readable());
        assert(!section->writable());
        assert(section->executable());
        assert(section->has_payload());
    }

    {
        LoadedImage image;

        LoadedImage::Section data;
        data.type = LoadedImage::SectionType::Data;
        data.flags =
            static_cast<std::int8_t>(LoadedImage::SectionFlags::Read) |
            static_cast<std::int8_t>(LoadedImage::SectionFlags::Write);
        data.virtual_address = 200;
        data.memory_size = 3;
        data.alignment = 1;
        data.words = {
            Word::from_integer(1),
            Word::from_integer(2),
            Word::from_integer(3)
        };

        image.sections().push_back(data);

        assert(image.data() != nullptr);
        assert(image.data()->readable());
        assert(image.data()->writable());
        assert(!image.data()->executable());
        assert(image.data()->has_payload());
        assert(image.data()->words.size() == 3);
    }

    {
        LoadedImage image;

        LoadedImage::Section rodata;
        rodata.type = LoadedImage::SectionType::Rodata;
        rodata.flags =
            static_cast<std::int8_t>(LoadedImage::SectionFlags::Read);
        rodata.virtual_address = 300;
        rodata.memory_size = 2;
        rodata.alignment = 1;
        rodata.words = {
            Word::from_integer(123),
            Word::from_integer(-456)
        };

        image.sections().push_back(rodata);

        assert(image.rodata() != nullptr);
        assert(image.rodata()->readable());
        assert(!image.rodata()->writable());
        assert(!image.rodata()->executable());
        assert(image.rodata()->has_payload());
    }

    {
        LoadedImage image;

        LoadedImage::Section bss;
        bss.type = LoadedImage::SectionType::Bss;
        bss.flags =
            static_cast<std::int8_t>(LoadedImage::SectionFlags::Read) |
            static_cast<std::int8_t>(LoadedImage::SectionFlags::Write);
        bss.virtual_address = 400;
        bss.memory_size = 64;
        bss.alignment = 1;

        image.sections().push_back(bss);

        assert(image.bss() != nullptr);
        assert(image.bss()->memory_size == 64);
        assert(image.bss()->words.empty());
        assert(!image.bss()->has_payload());
        assert(image.bss()->readable());
        assert(image.bss()->writable());
        assert(!image.bss()->executable());
    }

    {
        LoadedImage image;

        LoadedImage::Symbol symbol;
        symbol.identifier = 0;
        symbol.type = 2;
        symbol.section = 0;
        symbol.address = 100;

        image.symbols().push_back(symbol);

        assert(image.symbols().size() == 1);
        assert(image.symbols()[0].identifier == 0);
        assert(image.symbols()[0].type == 2);
        assert(image.symbols()[0].section == 0);
        assert(image.symbols()[0].address == 100);
    }

    {
        LoadedImage image;

        LoadedImage::Section text;
        text.type = LoadedImage::SectionType::Text;
        text.flags =
            static_cast<std::int8_t>(LoadedImage::SectionFlags::Read) |
            static_cast<std::int8_t>(LoadedImage::SectionFlags::Execute);
        text.virtual_address = 1000;
        text.memory_size = 4;
        text.words.resize(4, Word::zero());

        image.sections().push_back(text);

        image.set_entry_point(1000);
        assert(image.is_valid_entry_point());

        image.set_entry_point(1003);
        assert(image.is_valid_entry_point());

        image.set_entry_point(1004);
        assert(!image.is_valid_entry_point());

        image.set_entry_point(999);
        assert(!image.is_valid_entry_point());
    }

    {
        LoadedImage image;

        LoadedImage::Section text;
        text.type = LoadedImage::SectionType::Text;
        text.virtual_address = 1000;
        text.memory_size = 4;

        image.sections().push_back(text);

        image.set_entry_point(1000);
        assert(image.is_valid_entry_point());

        image.set_entry_point(1002);
        assert(image.is_valid_entry_point());
    }

    {
        LoadedImage image;

        image.set_entry_point(1000);

        assert(!image.is_valid_entry_point());
    }

    {
        LoadedImage image;

        LoadedImage::Section text;
        text.type = LoadedImage::SectionType::Text;
        text.virtual_address = 100;
        text.memory_size = 1;

        LoadedImage::Section data;
        data.type = LoadedImage::SectionType::Data;
        data.virtual_address = 200;
        data.memory_size = 2;

        image.sections().push_back(text);
        image.sections().push_back(data);

        assert(image.section(LoadedImage::SectionType::Text) != nullptr);
        assert(image.section(LoadedImage::SectionType::Data) != nullptr);
        assert(image.section(LoadedImage::SectionType::Rodata) == nullptr);
        assert(image.section(LoadedImage::SectionType::Bss) == nullptr);
    }

    {
        LoadedImage image;

        LoadedImage::Section text;
        text.type = LoadedImage::SectionType::Text;
        text.virtual_address = 100;
        text.memory_size = 1;

        image.sections().push_back(text);

        assert(image == image);

        LoadedImage copy = image;

        assert(copy == image);
    }

    return 0;
}

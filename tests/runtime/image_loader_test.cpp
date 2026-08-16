#include <cassert>
#include <cstdint>
#include <functional>
#include <stdexcept>

#include "runtime/image_loader.hpp"
#include "runtime/loaded_image.hpp"
#include "ternary/word.hpp"
#include "vm/machine.hpp"

using ternary_machine::runtime::ImageLoadError;
using ternary_machine::runtime::ImageLoader;
using ternary_machine::runtime::LoadedImage;
using ternary_machine::ternary::Word;
using ternary_machine::vm::Machine;

static bool rejects(const std::function<void()>& function) {
    try {
        function();
    } catch (const std::exception&) {
        return true;
    }

    return false;
}

static LoadedImage make_image() {
    LoadedImage image;

    image.set_architecture_id(1);
    image.set_isa_version(1);
    image.set_format_version(1);
    image.set_flags(0);
    image.set_memory_base(0);
    image.set_memory_size(16);
    image.set_entry_point(0);
    image.set_start_symbol(0);

    LoadedImage::Section text;
    text.type = LoadedImage::SectionType::Text;
    text.flags =
        static_cast<std::int8_t>(LoadedImage::SectionFlags::Read) |
        static_cast<std::int8_t>(LoadedImage::SectionFlags::Execute);
    text.virtual_address = 0;
    text.memory_size = 2;
    text.alignment = 1;
    text.words.push_back(Word::from_integer(1));
    text.words.push_back(Word::from_integer(2));

    LoadedImage::Section data;
    data.type = LoadedImage::SectionType::Data;
    data.flags =
        static_cast<std::int8_t>(LoadedImage::SectionFlags::Read) |
        static_cast<std::int8_t>(LoadedImage::SectionFlags::Write);
    data.virtual_address = 4;
    data.memory_size = 2;
    data.alignment = 1;
    data.words.push_back(Word::from_integer(42));
    data.words.push_back(Word::from_integer(-17));

    LoadedImage::Section rodata;
    rodata.type = LoadedImage::SectionType::Rodata;
    rodata.flags =
        static_cast<std::int8_t>(LoadedImage::SectionFlags::Read);
    rodata.virtual_address = 8;
    rodata.memory_size = 1;
    rodata.alignment = 1;
    rodata.words.push_back(Word::from_integer(99));

    LoadedImage::Section bss;
    bss.type = LoadedImage::SectionType::Bss;
    bss.flags =
        static_cast<std::int8_t>(LoadedImage::SectionFlags::Read) |
        static_cast<std::int8_t>(LoadedImage::SectionFlags::Write);
    bss.virtual_address = 12;
    bss.memory_size = 4;
    bss.alignment = 1;

    image.sections().push_back(text);
    image.sections().push_back(data);
    image.sections().push_back(rodata);
    image.sections().push_back(bss);

    return image;
}

int main() {
    {
        const auto image = make_image();
        Machine machine;

        ImageLoader{}.load(image, machine);

        assert(machine.cpu().pc() == Word::zero());
        assert(machine.memory().read(Word::from_integer(0)) == Word::from_integer(1));
        assert(machine.memory().read(Word::from_integer(1)) == Word::from_integer(2));
        assert(machine.memory().read(Word::from_integer(4)) == Word::from_integer(42));
        assert(machine.memory().read(Word::from_integer(5)) == Word::from_integer(-17));
        assert(machine.memory().read(Word::from_integer(8)) == Word::from_integer(99));
    }

    {
        const auto image = make_image();
        Machine machine;

        ImageLoader{}.load(image, machine);

        assert(machine.memory().read(Word::from_integer(12)) == Word::zero());
        assert(machine.memory().read(Word::from_integer(13)) == Word::zero());
        assert(machine.memory().read(Word::from_integer(14)) == Word::zero());
        assert(machine.memory().read(Word::from_integer(15)) == Word::zero());
    }

    {
        auto image = make_image();
        image.set_entry_point(1);

        Machine machine;

        ImageLoader{}.load(image, machine);

        assert(machine.cpu().pc() == Word::from_integer(1));
    }

    {
        auto image = make_image();
        image.set_architecture_id(99);

        Machine machine;

        assert(rejects([&] {
            ImageLoader{}.load(image, machine);
        }));
    }

    {
        auto image = make_image();
        image.set_isa_version(99);

        Machine machine;

        assert(rejects([&] {
            ImageLoader{}.load(image, machine);
        }));
    }

    {
        auto image = make_image();
        image.set_format_version(99);

        Machine machine;

        assert(rejects([&] {
            ImageLoader{}.load(image, machine);
        }));
    }

    {
        auto image = make_image();
        image.set_entry_point(16);

        Machine machine;

        assert(rejects([&] {
            ImageLoader{}.load(image, machine);
        }));
    }

    {
        auto image = make_image();
        image.sections()[0].memory_size = 3;

        Machine machine;

        assert(rejects([&] {
            ImageLoader{}.load(image, machine);
        }));
    }

    {
        auto image = make_image();
        image.sections()[3].words.push_back(Word::zero());

        Machine machine;

        assert(rejects([&] {
            ImageLoader{}.load(image, machine);
        }));
    }

    {
        auto image = make_image();
        image.sections()[0].virtual_address = 15;

        Machine machine;

        assert(rejects([&] {
            ImageLoader{}.load(image, machine);
        }));
    }

    {
        auto image = make_image();
        image.sections()[0].flags =
            static_cast<std::int8_t>(LoadedImage::SectionFlags::Read);

        Machine machine;

        assert(rejects([&] {
            ImageLoader{}.load(image, machine);
        }));
    }

    {
        auto image = make_image();

        Machine machine;
        machine.memory().write(Word::from_integer(0), Word::from_integer(999));
        machine.cpu().set_pc(Word::from_integer(7));

        ImageLoader{}.load(image, machine);

        assert(machine.memory().read(Word::from_integer(0)) == Word::from_integer(1));
        assert(machine.cpu().pc() == Word::zero());
    }

    return 0;
}

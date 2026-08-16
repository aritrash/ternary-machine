#include "runtime/image_loader.hpp"

#include <cstdint>
#include <limits>
#include <string>

namespace ternary_machine::runtime {

namespace {

[[nodiscard]] bool is_valid_section_type(LoadedImage::SectionType type) noexcept {
    switch (type) {
        case LoadedImage::SectionType::Text:
        case LoadedImage::SectionType::Data:
        case LoadedImage::SectionType::Rodata:
        case LoadedImage::SectionType::Bss:
            return true;
    }

    return false;
}

[[nodiscard]] std::int64_t section_end(const LoadedImage::Section& section) {
    if (section.virtual_address < 0 || section.memory_size < 0)
        throw ImageLoadError("section has a negative address or size");

    if (section.memory_size > std::numeric_limits<std::int64_t>::max() - section.virtual_address)
        throw ImageLoadError("section address range overflows");

    return section.virtual_address + section.memory_size;
}

}

void ImageLoader::validate(const LoadedImage& image) {
    if (image.architecture_id() != 1)
        throw ImageLoadError("unsupported TVM architecture identifier");

    if (image.isa_version() != 1)
        throw ImageLoadError("unsupported TVM ISA version");

    if (image.format_version() != 1)
        throw ImageLoadError("unsupported TRN executable format version");

    if (image.memory_base() < 0 || image.memory_size() < 0)
        throw ImageLoadError("invalid image memory layout");

    if (image.memory_size() > std::numeric_limits<std::int64_t>::max() - image.memory_base())
        throw ImageLoadError("image memory range overflows");

    const auto image_end = image.memory_base() + image.memory_size();

    const auto* text = image.text();

    if (text == nullptr)
        throw ImageLoadError("executable image has no .text section");

    if (!text->executable())
        throw ImageLoadError(".text section is not executable");

    if (!image.is_valid_entry_point())
        throw ImageLoadError("entry point is outside .text");

    for (const auto& section : image.sections()) {
        if (!is_valid_section_type(section.type))
            throw ImageLoadError("invalid section type");

        if (section.virtual_address < image.memory_base())
            throw ImageLoadError("section lies below image memory base");

        const auto end = section_end(section);

        if (end > image_end)
            throw ImageLoadError("section exceeds image memory range");

        if (section.alignment <= 0)
            throw ImageLoadError("section has invalid alignment");

        if (section.type == LoadedImage::SectionType::Bss) {
            if (!section.words.empty())
                throw ImageLoadError(".bss section contains physical payload");

            continue;
        }

        if (section.memory_size != static_cast<std::int64_t>(section.words.size()))
            throw ImageLoadError("section payload size does not match memory size");

        if (!section.writable() && section.type == LoadedImage::SectionType::Data)
            throw ImageLoadError(".data section is not writable");

        if (section.executable() && section.type != LoadedImage::SectionType::Text)
            throw ImageLoadError("non-text section is executable");
    }
}

void ImageLoader::load_section(const LoadedImage::Section& section, vm::Machine& machine) {
    if (section.type == LoadedImage::SectionType::Bss)
        return;

    for (std::size_t i = 0; i < section.words.size(); ++i) {
        const auto address = ternary::Word::from_integer(
            section.virtual_address + static_cast<std::int64_t>(i)
        );

        machine.memory().write(address, section.words[i]);
    }
}

void ImageLoader::load(const LoadedImage& image, vm::Machine& machine) const {
    validate(image);

    machine.reset();

    for (const auto& section : image.sections())
        load_section(section, machine);

    machine.cpu().set_pc(
        ternary::Word::from_integer(image.entry_point())
    );
}

}

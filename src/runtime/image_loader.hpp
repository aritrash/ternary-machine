#pragma once

#include <stdexcept>

#include "runtime/loaded_image.hpp"
#include "vm/machine.hpp"

namespace ternary_machine::runtime {

class ImageLoadError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class ImageLoader final {
public:
    void load(const LoadedImage& image, vm::Machine& machine) const;

private:
    static void validate(const LoadedImage& image);
    static void load_section(const LoadedImage::Section& section, vm::Machine& machine);
};

}

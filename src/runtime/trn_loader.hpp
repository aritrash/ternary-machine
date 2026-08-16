#pragma once

#include <stdexcept>
#include <string_view>

#include "runtime/loaded_image.hpp"

namespace ternary_machine::runtime {

class TrnLoaderError final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class TrnLoader final {
public:
    [[nodiscard]] LoadedImage load(std::string_view path) const;
};

}

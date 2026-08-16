#pragma once

#include <string_view>

#include "assembler/trn_image.hpp"
#include "ternary/word.hpp"

namespace ternary_machine::assembler {

class Assembler final {
public:
    [[nodiscard]] TrnImage assemble(std::string_view source, ternary::Word virtual_base = ternary::Word::zero()) const;

    void assemble_to_file(std::string_view source, std::string_view output_path, ternary::Word virtual_base = ternary::Word::zero()) const;
};

}

#pragma once

#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <string_view>

#include "trn_image.hpp"

namespace ternary_machine::assembler {

class TrnWriterError final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class TrnWriter final {
public:
    void write(const TrnImage& image, std::string_view path) const {
        if (path.empty())
            throw TrnWriterError("output path cannot be empty");

        if (!image.is_laid_out())
            throw TrnWriterError("cannot write an image before layout");

        std::ofstream output(std::string(path), std::ios::binary | std::ios::trunc);

        if (!output)
            throw TrnWriterError("failed to open output file");

        write_words(output, image.serialize());

        output.flush();

        if (!output)
            throw TrnWriterError("failed to finalize .trn image");
    }

private:
    static void write_words(std::ofstream& output, const std::vector<TrnFormat::Word>& words) {
        for (const auto& word : words) {
            output.write(word.data(), static_cast<std::streamsize>(word.size()));

            if (!output)
                throw TrnWriterError("failed while writing .trn image");
        }
    }
};

}

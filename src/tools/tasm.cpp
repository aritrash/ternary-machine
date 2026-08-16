#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "assembler/assembler.hpp"

namespace {

constexpr std::string_view VERSION = "0.1.0";

void print_help(std::ostream& out) {
    out << "TASM - Ternary Machine Assembler " << VERSION << '\n'
        << '\n'
        << "Usage:\n"
        << "  tasm <input.tasm> [-o <output.trn>]\n"
        << "  tasm --help\n"
        << "  tasm --version\n";
}

std::string default_output_path(const std::string& input) {
    const auto slash = input.find_last_of("/\\");
    const auto dot = input.find_last_of('.');

    if (dot != std::string::npos && (slash == std::string::npos || dot > slash))
        return input.substr(0, dot) + ".trn";

    return input + ".trn";
}

}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cerr << "tasm: error: no input file specified\n";
        std::cerr << "tasm: use --help for usage information\n";
        return 1;
    }

    if (std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h") {
        print_help(std::cout);
        return 0;
    }

    if (std::string_view(argv[1]) == "--version" || std::string_view(argv[1]) == "-v") {
        std::cout << "TASM " << VERSION << '\n';
        return 0;
    }

    std::string input_path = argv[1];
    std::string output_path;

    for (int i = 2; i < argc; ++i) {
        const std::string_view argument = argv[i];

        if (argument == "-o" || argument == "--output") {
            if (++i >= argc) {
                std::cerr << "tasm: error: missing output path after " << argument << '\n';
                return 1;
            }

            output_path = argv[i];
            continue;
        }

        std::cerr << "tasm: error: unknown argument '" << argument << "'\n";
        return 1;
    }

    if (output_path.empty())
        output_path = default_output_path(input_path);

    std::ifstream input(input_path);

    if (!input) {
        std::cerr << "tasm: error: cannot open input file '" << input_path << "'\n";
        return 1;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();

    try {
        ternary_machine::assembler::Assembler assembler;
        assembler.assemble_to_file(buffer.str(), output_path);

        std::cout << "tasm: assembled '" << input_path
                  << "' -> '" << output_path << "'\n";

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "tasm: error: " << error.what() << '\n';
        return 1;
    }
}

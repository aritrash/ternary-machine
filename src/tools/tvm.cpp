#include <exception>
#include <iostream>
#include <string_view>

#include "runtime/trn_loader.hpp"
#include "runtime/tvm.hpp"

using ternary_machine::runtime::TrnLoader;
using ternary_machine::runtime::TVM;

namespace {

void print_usage(std::string_view program) {
    std::cerr << "Usage: " << program << " <program.trn>\n";
}

}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        print_usage(argc > 0 ? argv[0] : "tvm");
        return 2;
    }

    try {
        const auto image = TrnLoader{}.load(argv[1]);

        TVM tvm;
        tvm.load(image);
        tvm.run();

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "tvm: " << error.what() << '\n';
        return 1;
    }
}

#include <cassert>
#include <filesystem>

#include "assembler/assembler.hpp"
#include "assembler/trn_writer.hpp"
#include "runtime/image_loader.hpp"
#include "runtime/trn_loader.hpp"
#include "vm/executor.hpp"
#include "vm/machine.hpp"
#include "vm/register_file.hpp"

using ternary_machine::assembler::Assembler;
using ternary_machine::assembler::TrnWriter;
using ternary_machine::runtime::ImageLoader;
using ternary_machine::runtime::TrnLoader;
using ternary_machine::vm::Executor;
using ternary_machine::vm::Machine;
using ternary_machine::vm::Register;

int main() {
    const auto path = std::filesystem::temp_directory_path() / "ternary_machine_execution.trn";

    const auto source =
        "section .text\n"
        "global _start\n"
        "_start:\n"
        "LDI R1, 10\n"
        "LDI R2, 20\n"
        "ADD R3, R1, R2\n"
        "HLT\n";

    Assembler assembler;
    const auto image = assembler.assemble(source);

    TrnWriter{}.write(image, path.string());

    const auto loaded = TrnLoader{}.load(path.string());

    Machine machine;
    ImageLoader{}.load(loaded, machine);

    Executor executor;

    while (!machine.halted())
        executor.step(machine);

    assert(machine.cpu().registers().read(Register::R1).to_integer() == 10);
    assert(machine.cpu().registers().read(Register::R2).to_integer() == 20);
    assert(machine.cpu().registers().read(Register::R3).to_integer() == 30);
    assert(machine.halted());

    std::filesystem::remove(path);

    return 0;
}

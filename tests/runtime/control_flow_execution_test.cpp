#include <cassert>
#include <cstdio>
#include <string>

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
    constexpr const char* path = "control_flow_test.trn";

    const std::string source =
        "section .text\n"
        "global _start\n"
        "_start:\n"
        "LDI R1, 10\n"
        "LDI R2, 10\n"
        "CMP R1, R2\n"
        "BEQ equal\n"
        "LDI R3, 0\n"
        "HLT\n"
        "equal:\n"
        "LDI R3, 1\n"
        "HLT\n";

    Assembler assembler;
    const auto image = assembler.assemble(source);

    assert(image.is_laid_out());
    assert(image.text().size() == 8);
    assert(image.header().entry_point == 0);

    TrnWriter{}.write(image, path);

    const auto loaded = TrnLoader{}.load(path);

    assert(loaded.architecture_id() == 1);
    assert(loaded.isa_version() == 1);
    assert(loaded.format_version() == 1);
    assert(loaded.entry_point() == 0);

    const auto* text = loaded.text();
    assert(text != nullptr);
    assert(text->words.size() == 8);
    assert(text->virtual_address == 0);
    assert(text->memory_size == 8);
    assert(text->executable());

    Machine machine;
    ImageLoader{}.load(loaded, machine);

    assert(!machine.halted());
    assert(machine.cpu().pc().to_integer() == 0);

    Executor executor;

    std::size_t steps = 0;
    constexpr std::size_t max_steps = 16;

    while (!machine.halted() && steps < max_steps) {
        executor.step(machine);
        ++steps;
    }

    assert(machine.halted());
    assert(steps < max_steps);

    assert(machine.cpu().registers().read(Register::R1).to_integer() == 10);
    assert(machine.cpu().registers().read(Register::R2).to_integer() == 10);
    assert(machine.cpu().registers().read(Register::R3).to_integer() == 1);

    assert(machine.cpu().pc().to_integer() == 7);

    std::remove(path);

    return 0;
}

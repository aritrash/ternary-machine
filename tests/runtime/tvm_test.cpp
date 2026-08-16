#include <cassert>
#include <cstdio>

#include "assembler/assembler.hpp"
#include "assembler/trn_writer.hpp"
#include "runtime/trn_loader.hpp"
#include "runtime/tvm.hpp"
#include "ternary/word.hpp"
#include "vm/register_file.hpp"

using ternary_machine::assembler::Assembler;
using ternary_machine::assembler::TrnWriter;
using ternary_machine::runtime::TrnLoader;
using ternary_machine::runtime::TVM;
using ternary_machine::ternary::Word;
using ternary_machine::vm::Register;

int main() {
    constexpr const char* path = "tvm_test.trn";

    {
        const auto image = Assembler{}.assemble(
            "section .text\n"
            "_start:\n"
            "HLT\n"
        );

        TrnWriter{}.write(image, path);

        const auto loaded = TrnLoader{}.load(path);

        TVM tvm;
        tvm.load(loaded);

        assert(tvm.machine().cpu().pc() == Word::zero());
        assert(!tvm.halted());

        tvm.step();

        assert(tvm.halted());
    }

    {
        const auto image = Assembler{}.assemble(
            "section .text\n"
            "_start:\n"
            "LDI R1, 42\n"
            "HLT\n"
        );

        TrnWriter{}.write(image, path);

        const auto loaded = TrnLoader{}.load(path);

        TVM tvm;
        tvm.load(loaded);

        assert(!tvm.halted());

        tvm.step();

        assert(!tvm.halted());
        assert(tvm.machine().cpu().registers().read(Register::R1).to_integer() == 42);

        tvm.step();

        assert(tvm.halted());
        assert(tvm.machine().cpu().registers().read(Register::R1).to_integer() == 42);
    }

    {
        const auto image = Assembler{}.assemble(
            "section .text\n"
            "_start:\n"
            "LDI R1, 10\n"
            "LDI R2, 32\n"
            "ADD R3, R1, R2\n"
            "HLT\n"
        );

        TrnWriter{}.write(image, path);

        const auto loaded = TrnLoader{}.load(path);

        TVM tvm;
        tvm.load(loaded);

        tvm.run();

        assert(tvm.halted());
        assert(tvm.machine().cpu().registers().read(Register::R1).to_integer() == 10);
        assert(tvm.machine().cpu().registers().read(Register::R2).to_integer() == 32);
        assert(tvm.machine().cpu().registers().read(Register::R3).to_integer() == 42);
    }

    {
        const auto image = Assembler{}.assemble(
            "section .text\n"
            "_start:\n"
            "LDI R1, 3\n"
            "LDI R2, 1\n"
            "loop:\n"
            "SUB R1, R1, R2\n"
            "CMP R1, R0\n"
            "BGT loop\n"
            "HLT\n"
        );

        TrnWriter{}.write(image, path);

        const auto loaded = TrnLoader{}.load(path);

        TVM tvm;
        tvm.load(loaded);

        tvm.run();

        assert(tvm.halted());
        assert(tvm.machine().cpu().registers().read(Register::R1).to_integer() == 0);
    }

    std::remove(path);

    return 0;
}

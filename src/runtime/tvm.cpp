#include "runtime/tvm.hpp"

#include "runtime/image_loader.hpp"
#include "vm/executor.hpp"

namespace ternary_machine::runtime {

void TVM::load(const LoadedImage& image) {
    ImageLoader{}.load(image, machine_);
}

void TVM::step() {
    if (machine_.halted())
        return;

    vm::Executor{}.step(machine_);
}

void TVM::run() {
    while (!machine_.halted())
        step();
}

const vm::Machine& TVM::machine() const noexcept {
    return machine_;
}

vm::Machine& TVM::machine() noexcept {
    return machine_;
}

bool TVM::halted() const noexcept {
    return machine_.halted();
}

}

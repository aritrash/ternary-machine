#pragma once

#include "runtime/loaded_image.hpp"
#include "vm/machine.hpp"

namespace ternary_machine::runtime {

class TVM final {
public:
    TVM() noexcept = default;

    void load(const LoadedImage& image);
    void step();
    void run();

    [[nodiscard]] const vm::Machine& machine() const noexcept;
    [[nodiscard]] vm::Machine& machine() noexcept;
    [[nodiscard]] bool halted() const noexcept;

private:
    vm::Machine machine_{};
};

}

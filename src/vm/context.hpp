#pragma once

#include "cpu_state.hpp"

namespace ternary_machine::vm {

struct SavedContext final {
    RegisterFile registers{};
    ternary::Word pc{};
    ternary::Word sp{};
    Comparison status = Comparison::Equal;
    PrivilegeLevel privilege = PrivilegeLevel::User;
    TransitionCause cause = TransitionCause::SystemCall;

    [[nodiscard]] static SavedContext capture(const CPUState& cpu, TransitionCause transition_cause) noexcept {
        SavedContext context;
        context.registers = cpu.registers();
        context.pc = cpu.pc();
        context.sp = cpu.sp();
        context.status = cpu.status();
        context.privilege = cpu.privilege();
        context.cause = transition_cause;
        return context;
    }

    void restore(CPUState& cpu) const noexcept {
        cpu.registers() = registers;
        cpu.set_pc(pc);
        cpu.set_sp(sp);
        cpu.set_status(status);
        cpu.set_privilege(privilege);
    }
};

}

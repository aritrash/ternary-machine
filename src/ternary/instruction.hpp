#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>

#include "word.hpp"

namespace ternary_machine::ternary {

enum class Opcode : std::int8_t {
    NOP = -13,
    ADD = -12,
    SUB = -11,
    MUL = -10,
    CMP = -9,
    TAND = -8,
    TOR = -7,
    TXOR = -6,
    TNOT = -5,
    SHF = -4,
    LDI = -3,
    MOV = -2,
    LD = -1,
    ST = 0,
    LEA = 1,
    JMP = 2,
    BEQ = 3,
    BGT = 4,
    BLT = 5,
    CALL = 6,
    RET = 7,
    IN = 8,
    OUT = 9,
    SYS = 10,
    IRET = 11,
    SWAP = 12,
    HLT = 13
};

class Instruction final {
public:
    static constexpr std::size_t OPCODE_OFFSET = 0;
    static constexpr std::size_t OPCODE_WIDTH = 3;
    static constexpr std::size_t RD_OFFSET = 3;
    static constexpr std::size_t RD_WIDTH = 2;
    static constexpr std::size_t RS1_OFFSET = 5;
    static constexpr std::size_t RS1_WIDTH = 2;
    static constexpr std::size_t RS2_OFFSET = 7;
    static constexpr std::size_t RS2_WIDTH = 2;
    static constexpr std::size_t CARGO_OFFSET = 9;
    static constexpr std::size_t CARGO_WIDTH = 18;
    static constexpr std::size_t WIDTH = 27;

    using Cargo = std::array<Trit, CARGO_WIDTH>;

    constexpr Instruction() noexcept = default;

    static Instruction encode(Opcode opcode, std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2, const Cargo& cargo = {}) {
        if (rd > 8 || rs1 > 8 || rs2 > 8) throw std::out_of_range("invalid TVM register");

        Instruction instruction;
        instruction.set_opcode(opcode);
        instruction.set_register(RD_OFFSET, rd);
        instruction.set_register(RS1_OFFSET, rs1);
        instruction.set_register(RS2_OFFSET, rs2);

        for (std::size_t i = 0; i < CARGO_WIDTH; ++i)
            instruction.word_.set_trit(CARGO_OFFSET + i, cargo[i]);

        return instruction;
    }
    
    static constexpr Instruction decode(const Word& word) noexcept {
	    Instruction instruction;
	    instruction.word_ = word;
	    return instruction;
	}

    [[nodiscard]] constexpr Opcode opcode() const noexcept {
        return static_cast<Opcode>(decode_balanced(OPCODE_OFFSET, OPCODE_WIDTH));
    }

    [[nodiscard]] constexpr std::uint8_t rd() const noexcept {
        return decode_register(RD_OFFSET);
    }

    [[nodiscard]] constexpr std::uint8_t rs1() const noexcept {
        return decode_register(RS1_OFFSET);
    }

    [[nodiscard]] constexpr std::uint8_t rs2() const noexcept {
        return decode_register(RS2_OFFSET);
    }

    [[nodiscard]] constexpr Cargo cargo() const noexcept {
        Cargo result{};
        for (std::size_t i = 0; i < CARGO_WIDTH; ++i)
            result[i] = word_.trit(CARGO_OFFSET + i);
        return result;
    }

    [[nodiscard]] constexpr const Word& word() const noexcept {
        return word_;
    }

    constexpr bool operator==(const Instruction&) const noexcept = default;
    
private:
    static constexpr std::array<std::array<Trit, 2>, 9> REGISTER_ENCODING = {{
        {{Trit::Neg, Trit::Neg}},
        {{Trit::Neg, Trit::Zero}},
        {{Trit::Neg, Trit::Pos}},
        {{Trit::Zero, Trit::Neg}},
        {{Trit::Zero, Trit::Zero}},
        {{Trit::Zero, Trit::Pos}},
        {{Trit::Pos, Trit::Neg}},
        {{Trit::Pos, Trit::Zero}},
        {{Trit::Pos, Trit::Pos}}
    }};

    constexpr void set_opcode(Opcode opcode) noexcept {
        std::int8_t value = static_cast<std::int8_t>(opcode);

        for (std::size_t i = OPCODE_WIDTH; i-- > 0;) {
            const std::int8_t remainder = value % 3;
            Trit trit;

            if (remainder == 2) {
                trit = Trit::Neg;
                value = static_cast<std::int8_t>((value + 1) / 3);
            } else if (remainder == -2) {
                trit = Trit::Pos;
                value = static_cast<std::int8_t>((value - 1) / 3);
            } else {
                trit = static_cast<Trit>(remainder);
                value = static_cast<std::int8_t>(value / 3);
            }

            word_.set_trit(i, trit);
        }
    }

    constexpr void set_register(std::size_t offset, std::uint8_t reg) noexcept {
        word_.set_trit(offset, REGISTER_ENCODING[reg][0]);
        word_.set_trit(offset + 1, REGISTER_ENCODING[reg][1]);
    }

    constexpr std::uint8_t decode_register(std::size_t offset) const noexcept {
        const Trit first = word_.trit(offset);
        const Trit second = word_.trit(offset + 1);

        for (std::uint8_t reg = 0; reg < REGISTER_ENCODING.size(); ++reg)
            if (REGISTER_ENCODING[reg][0] == first && REGISTER_ENCODING[reg][1] == second)
                return reg;

        return 0;
    }

    constexpr std::int8_t decode_balanced(std::size_t offset, std::size_t width) const noexcept {
        std::int8_t value = 0;

        for (std::size_t i = 0; i < width; ++i)
            value = static_cast<std::int8_t>(value * 3 + static_cast<std::int8_t>(word_.trit(offset + i)));

        return value;
    }

    Word word_{};
};

}

#include <cassert>
#include <cstdint>
#include <string>
#include <variant>

#include "assembler/lexer.hpp"
#include "assembler/parser.hpp"

using ternary_machine::assembler::AssemblyProgram;
using ternary_machine::assembler::GlobalIR;
using ternary_machine::assembler::ImmediateOperand;
using ternary_machine::assembler::InstructionIR;
using ternary_machine::assembler::LabelIR;
using ternary_machine::assembler::Lexer;
using ternary_machine::assembler::MemoryOperand;
using ternary_machine::assembler::ParseError;
using ternary_machine::assembler::Parser;
using ternary_machine::assembler::RegisterOperand;
using ternary_machine::assembler::SectionIR;
using ternary_machine::assembler::SymbolOperand;

static AssemblyProgram parse(const char* source) {
    const auto tokens = Lexer(source).tokenize();
    return Parser(tokens).parse();
}

int main() {
    {
        const auto program = parse("HLT\n");

        assert(program.statements.size() == 1);
        assert(std::holds_alternative<InstructionIR>(program.statements[0]));

        const auto& instruction = std::get<InstructionIR>(program.statements[0]);

        assert(instruction.mnemonic == "HLT");
        assert(instruction.operands.empty());
        assert(instruction.location.line == 1);
        assert(instruction.location.column == 1);
    }

    {
        const auto program = parse("ADD R3, R1, R2\n");

        assert(program.statements.size() == 1);

        const auto& instruction = std::get<InstructionIR>(program.statements[0]);

        assert(instruction.mnemonic == "ADD");
        assert(instruction.operands.size() == 3);

        assert(std::get<RegisterOperand>(instruction.operands[0]).index == 3);
        assert(std::get<RegisterOperand>(instruction.operands[1]).index == 1);
        assert(std::get<RegisterOperand>(instruction.operands[2]).index == 2);
    }

    {
        const auto program = parse("LDI R1, 42\n");

        const auto& instruction = std::get<InstructionIR>(program.statements[0]);

        assert(instruction.mnemonic == "LDI");
        assert(instruction.operands.size() == 2);
        assert(std::get<RegisterOperand>(instruction.operands[0]).index == 1);
        assert(std::get<ImmediateOperand>(instruction.operands[1]).value == 42);
    }

    {
        const auto program = parse("LDI R1, -12345\n");

        const auto& instruction = std::get<InstructionIR>(program.statements[0]);

        assert(instruction.operands.size() == 2);
        assert(std::get<ImmediateOperand>(instruction.operands[1]).value == -12345);
    }

    {
        const auto program = parse("JMP _start\n");

        const auto& instruction = std::get<InstructionIR>(program.statements[0]);

        assert(instruction.mnemonic == "JMP");
        assert(instruction.operands.size() == 1);
        assert(std::get<SymbolOperand>(instruction.operands[0]).name == "_start");
    }

    {
        const auto program = parse("LD R1, [R2]\n");

        const auto& instruction = std::get<InstructionIR>(program.statements[0]);

        assert(instruction.mnemonic == "LD");
        assert(instruction.operands.size() == 2);

        const auto& memory = std::get<MemoryOperand>(instruction.operands[1]);

        assert(memory.base == 2);
        assert(memory.offset == 0);
    }

    {
        const auto program = parse("LD R1, [R2 + 10]\n");

        const auto& instruction = std::get<InstructionIR>(program.statements[0]);

        assert(instruction.operands.size() == 2);

        const auto& memory = std::get<MemoryOperand>(instruction.operands[1]);

        assert(memory.base == 2);
        assert(memory.offset == 10);
    }

    {
        const auto program = parse("LD R1, [R2 + -10]\n");

        const auto& instruction = std::get<InstructionIR>(program.statements[0]);

        const auto& memory = std::get<MemoryOperand>(instruction.operands[1]);

        assert(memory.base == 2);
        assert(memory.offset == -10);
    }

    {
        const auto program = parse(
            "section .text\n"
            "global _start\n"
            "_start:\n"
            "HLT\n"
        );

        assert(program.statements.size() == 4);

        assert(std::holds_alternative<SectionIR>(program.statements[0]));
        assert(std::holds_alternative<GlobalIR>(program.statements[1]));
        assert(std::holds_alternative<LabelIR>(program.statements[2]));
        assert(std::holds_alternative<InstructionIR>(program.statements[3]));

        assert(std::get<SectionIR>(program.statements[0]).name == "text");
        assert(std::get<GlobalIR>(program.statements[1]).name == "_start");
        assert(std::get<LabelIR>(program.statements[2]).name == "_start");
        assert(std::get<InstructionIR>(program.statements[3]).mnemonic == "HLT");
    }

    {
        const auto program = parse(
            "\n"
            "\n"
            "section .text\n"
            "\n"
            "_start:\n"
            "\n"
            "HLT\n"
            "\n"
        );

        assert(program.statements.size() == 3);
        assert(std::holds_alternative<SectionIR>(program.statements[0]));
        assert(std::holds_alternative<LabelIR>(program.statements[1]));
        assert(std::holds_alternative<InstructionIR>(program.statements[2]));
    }

    {
        const auto program = parse("MOV R1, R2\nADD R3, R1, R2\n");

        assert(program.statements.size() == 2);
        assert(std::get<InstructionIR>(program.statements[0]).mnemonic == "MOV");
        assert(std::get<InstructionIR>(program.statements[1]).mnemonic == "ADD");
    }

    {
        bool threw = false;

        try {
            parse("ADD R1 R2, R3\n");
        } catch (const ParseError&) {
            threw = true;
        }

        assert(threw);
    }

    {
        bool threw = false;

        try {
            parse("LD R1, [R2\n");
        } catch (const ParseError&) {
            threw = true;
        }

        assert(threw);
    }

    {
        bool threw = false;

        try {
            parse("LD R1, []\n");
        } catch (const ParseError&) {
            threw = true;
        }

        assert(threw);
    }

    {
        bool threw = false;

        try {
            parse("section text\n");
        } catch (const ParseError&) {
            threw = true;
        }

        assert(threw);
    }

    {
        bool threw = false;

        try {
            parse("global\n");
        } catch (const ParseError&) {
            threw = true;
        }

        assert(threw);
    }
    
    {
		const auto program = parse("LD R1, [R2 - 10]\n");

		const auto& instruction = std::get<InstructionIR>(program.statements[0]);

		assert(instruction.mnemonic == "LD");
		assert(instruction.operands.size() == 2);

		const auto& memory = std::get<MemoryOperand>(instruction.operands[1]);

		assert(memory.base == 2);
		assert(memory.offset == -10);
	}
	
	{
		bool threw = false;

		try {
		    parse("LD R1, [R2 -]\n");
		} catch (const ParseError&) {
		    threw = true;
		}

		assert(threw);
	}

	{
		bool threw = false;

		try {
		    parse("LD R1, [R2 +]\n");
		} catch (const ParseError&) {
		    threw = true;
		}

		assert(threw);
	}

    return 0;
}

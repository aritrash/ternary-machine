#include <cassert>
#include <string>

#include "assembler/lexer.hpp"
#include "assembler/parser.hpp"
#include "assembler/semantic.hpp"

using ternary_machine::assembler::Lexer;
using ternary_machine::assembler::Parser;
using ternary_machine::assembler::ParseError;
using ternary_machine::assembler::SemanticAnalyzer;
using ternary_machine::assembler::SemanticError;

static void analyze(const char* source) {
    const auto tokens = Lexer(source).tokenize();
    const auto program = Parser(tokens).parse();
    SemanticAnalyzer{}.analyze(program);
}

static bool rejects(const char* source) {
    try {
        analyze(source);
    } catch (const SemanticError&) {
        return true;
    } catch (const ParseError&) {
        return true;
    }

    return false;
}

int main() {
    {
        analyze("NOP\n");
        analyze("HLT\n");
        analyze("RET\n");
        analyze("SYS\n");
        analyze("IRET\n");
        analyze("SWAP\n");
    }

    {
        analyze("ADD R1, R2, R3\n");
        analyze("SUB R1, R2, R3\n");
        analyze("MUL R1, R2, R3\n");
        analyze("TAND R1, R2, R3\n");
        analyze("TOR R1, R2, R3\n");
        analyze("TXOR R1, R2, R3\n");
    }

    {
        analyze("CMP R1, R2\n");
        analyze("TNOT R1, R2\n");
        analyze("MOV R1, R2\n");
    }

    {
        analyze("LDI R1, 42\n");
        analyze("LDI R8, -123\n");
        analyze("SHF R1, R2, R3\n");
    }

    {
        analyze("LD R1, [R2]\n");
        analyze("LD R1, [R2 + 10]\n");
        analyze("ST [R1], R2\n");
		analyze("ST [R2 + 10], R1\n");
		analyze("ST [R2 - 10], R1\n");
        analyze("LEA R1, [R2 + 27]\n");
    }

    {
        analyze("JMP _start\n");
        analyze("BEQ loop\n");
        analyze("BGT greater\n");
        analyze("BLT less\n");
        analyze("CALL function\n");
    }

    {
        analyze("IN R1, 0\n");
        analyze("OUT R1, 1\n");
    }

    {
        analyze(
            "section .text\n"
            "global _start\n"
            "_start:\n"
            "LDI R1, 10\n"
            "LDI R2, 20\n"
            "ADD R3, R1, R2\n"
            "HLT\n"
        );
    }

    {
        assert(rejects("MEOW R1, R2\n"));
        assert(rejects("FOOBAR\n"));
    }

    {
        assert(rejects("ADD R1, R2\n"));
        assert(rejects("ADD R1, R2, R3, R4\n"));
        assert(rejects("HLT R1\n"));
        assert(rejects("LDI R1\n"));
        assert(rejects("RET R1\n"));
    }

    {
        assert(rejects("ADD R1, 10, R3\n"));
        assert(rejects("LDI R1, R2\n"));
        assert(rejects("MOV R1, 10\n"));
        assert(rejects("JMP R1\n"));
        assert(rejects("CMP R1, 10\n"));
        assert(rejects("SHF R1, R2, 3\n"));
    }

    {
        assert(rejects("LD R1, R2\n"));
        assert(rejects("LD R1, [R2 + R3]\n"));
        assert(rejects("LD R1, [R2 + 1 + 2]\n"));
        assert(rejects("LD R1, []\n"));
        assert(rejects("ST R1, [R2]\n"));
        assert(rejects("ST [R2], 10\n"));
    }

    {
        assert(rejects("global 123\n"));
        assert(rejects("section .123\n"));
    }

    {
		analyze("_start:\n");
		analyze("loop1:\n");
		analyze("_R0:\n");
		analyze("R8_label:\n");
	}

    return 0;
}

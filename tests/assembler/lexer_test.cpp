#include <cassert>
#include <cstdint>
#include <string>

#include "assembler/lexer.hpp"

using ternary_machine::assembler::LexError;
using ternary_machine::assembler::Lexer;
using ternary_machine::assembler::Token;
using ternary_machine::assembler::TokenKind;

static void assert_token(const Token& token, TokenKind kind, const std::string& text = "") {
    assert(token.kind == kind);
    if (!text.empty())
        assert(token.text == text);
}

int main() {
    {
        const auto tokens = Lexer("ADD R3, R1, R2\n").tokenize();

        assert(tokens.size() == 8);

        assert_token(tokens[0], TokenKind::Identifier, "ADD");
        assert_token(tokens[1], TokenKind::Register);
        assert(tokens[1].register_index == 3);
        assert_token(tokens[2], TokenKind::Comma);
        assert_token(tokens[3], TokenKind::Register);
        assert(tokens[3].register_index == 1);
        assert_token(tokens[4], TokenKind::Comma);
        assert_token(tokens[5], TokenKind::Register);
        assert(tokens[5].register_index == 2);
        assert_token(tokens[6], TokenKind::Newline);
        assert_token(tokens[7], TokenKind::EndOfFile);
    }

    {
        const auto tokens = Lexer("LDI R1, 42\nLDI R2, -17\n").tokenize();

        assert(tokens.size() == 11);

        assert_token(tokens[0], TokenKind::Identifier, "LDI");
        assert_token(tokens[1], TokenKind::Register);
        assert(tokens[1].register_index == 1);
        assert_token(tokens[2], TokenKind::Comma);
        assert_token(tokens[3], TokenKind::Number);
        assert(tokens[3].number == 42);
        assert_token(tokens[4], TokenKind::Newline);

        assert_token(tokens[5], TokenKind::Identifier, "LDI");
        assert_token(tokens[6], TokenKind::Register);
        assert(tokens[6].register_index == 2);
        assert_token(tokens[7], TokenKind::Comma);
        assert_token(tokens[8], TokenKind::Number);
        assert(tokens[8].number == -17);
        assert_token(tokens[9], TokenKind::Newline);
        assert_token(tokens[10], TokenKind::EndOfFile);
    }

    {
        const auto tokens = Lexer("LD R1, [R2 + 10]\n").tokenize();

        assert(tokens.size() == 10);

        assert_token(tokens[0], TokenKind::Identifier, "LD");
        assert_token(tokens[1], TokenKind::Register);
        assert(tokens[1].register_index == 1);
        assert_token(tokens[2], TokenKind::Comma);
        assert_token(tokens[3], TokenKind::LBracket);
        assert_token(tokens[4], TokenKind::Register);
        assert(tokens[4].register_index == 2);
        assert_token(tokens[5], TokenKind::Plus);
        assert_token(tokens[6], TokenKind::Number);
        assert(tokens[6].number == 10);
        assert_token(tokens[7], TokenKind::RBracket);
        assert_token(tokens[8], TokenKind::Newline);
        assert_token(tokens[9], TokenKind::EndOfFile);
    }

    {
        const auto tokens = Lexer("ST [R3], R4\n").tokenize();

        assert(tokens.size() == 8);

        assert_token(tokens[0], TokenKind::Identifier, "ST");
        assert_token(tokens[1], TokenKind::LBracket);
        assert_token(tokens[2], TokenKind::Register);
        assert(tokens[2].register_index == 3);
        assert_token(tokens[3], TokenKind::RBracket);
        assert_token(tokens[4], TokenKind::Comma);
        assert_token(tokens[5], TokenKind::Register);
        assert(tokens[5].register_index == 4);
        assert_token(tokens[6], TokenKind::Newline);
        assert_token(tokens[7], TokenKind::EndOfFile);
    }

    {
        const auto tokens = Lexer(
            "section .text\n"
            "global _start\n"
            "_start:\n"
            "    HLT\n"
        ).tokenize();

        assert(tokens.size() == 13);

        assert_token(tokens[0], TokenKind::Identifier, "section");
        assert_token(tokens[1], TokenKind::Dot);
        assert_token(tokens[2], TokenKind::Identifier, "text");
        assert_token(tokens[3], TokenKind::Newline);

        assert_token(tokens[4], TokenKind::Identifier, "global");
        assert_token(tokens[5], TokenKind::Identifier, "_start");
        assert_token(tokens[6], TokenKind::Newline);

        assert_token(tokens[7], TokenKind::Identifier, "_start");
        assert_token(tokens[8], TokenKind::Colon);
        assert_token(tokens[9], TokenKind::Newline);

        assert_token(tokens[10], TokenKind::Identifier, "HLT");
        assert_token(tokens[11], TokenKind::Newline);
        assert_token(tokens[12], TokenKind::EndOfFile);
    }

    {
        const auto tokens = Lexer(
            "section .text\n"
            "\n"
            "\n"
            "_start:\n"
            "    HLT\n"
        ).tokenize();

        assert(tokens.size() == 12);
        assert_token(tokens[0], TokenKind::Identifier, "section");
        assert_token(tokens[1], TokenKind::Dot);
        assert_token(tokens[2], TokenKind::Identifier, "text");
        assert_token(tokens[3], TokenKind::Newline);
        assert_token(tokens[4], TokenKind::Newline);
        assert_token(tokens[5], TokenKind::Newline);
        assert_token(tokens[6], TokenKind::Identifier, "_start");
        assert_token(tokens[7], TokenKind::Colon);
        assert_token(tokens[8], TokenKind::Newline);
        assert_token(tokens[9], TokenKind::Identifier, "HLT");
        assert_token(tokens[10], TokenKind::Newline);
        assert_token(tokens[11], TokenKind::EndOfFile);
    }

    {
        const auto tokens = Lexer(
            "; comment\n"
            "LDI R1, 10 ; inline comment\n"
            "HLT\n"
        ).tokenize();

        assert(tokens.size() == 9);

        assert_token(tokens[0], TokenKind::Newline);

        assert_token(tokens[1], TokenKind::Identifier, "LDI");
        assert_token(tokens[2], TokenKind::Register);
        assert(tokens[2].register_index == 1);
        assert_token(tokens[3], TokenKind::Comma);
        assert_token(tokens[4], TokenKind::Number);
        assert(tokens[4].number == 10);
        assert_token(tokens[5], TokenKind::Newline);

        assert_token(tokens[6], TokenKind::Identifier, "HLT");
        assert_token(tokens[7], TokenKind::Newline);
        assert_token(tokens[8], TokenKind::EndOfFile);
    }

    {
        const auto tokens = Lexer("r0 R1 r2 R3 r4 R5 r6 R7 r8\n").tokenize();

        for (std::uint8_t i = 0; i < 9; ++i) {
            assert_token(tokens[i], TokenKind::Register);
            assert(tokens[i].register_index == i);
        }

        assert_token(tokens[9], TokenKind::Newline);
        assert_token(tokens[10], TokenKind::EndOfFile);
    }

    {
        const auto tokens = Lexer("R9 R10 R99\n").tokenize();

        assert_token(tokens[0], TokenKind::Identifier, "R9");
        assert_token(tokens[1], TokenKind::Identifier, "R10");
        assert_token(tokens[2], TokenKind::Identifier, "R99");
        assert_token(tokens[3], TokenKind::Newline);
        assert_token(tokens[4], TokenKind::EndOfFile);
    }

    {
        const auto tokens = Lexer("ADD\tR1,\tR2\t,\tR3\r\n").tokenize();

        assert_token(tokens[0], TokenKind::Identifier, "ADD");
        assert_token(tokens[1], TokenKind::Register);
        assert(tokens[1].register_index == 1);
        assert_token(tokens[2], TokenKind::Comma);
        assert_token(tokens[3], TokenKind::Register);
        assert(tokens[3].register_index == 2);
        assert_token(tokens[4], TokenKind::Comma);
        assert_token(tokens[5], TokenKind::Register);
        assert(tokens[5].register_index == 3);
        assert_token(tokens[6], TokenKind::Newline);
        assert_token(tokens[7], TokenKind::EndOfFile);
    }

    {
        const auto tokens = Lexer("loop:\nnext_label:\n").tokenize();

        assert_token(tokens[0], TokenKind::Identifier, "loop");
        assert_token(tokens[1], TokenKind::Colon);
        assert_token(tokens[2], TokenKind::Newline);

        assert_token(tokens[3], TokenKind::Identifier, "next_label");
        assert_token(tokens[4], TokenKind::Colon);
        assert_token(tokens[5], TokenKind::Newline);

        assert_token(tokens[6], TokenKind::EndOfFile);
    }

    {
        bool threw = false;

        try {
            (void)Lexer("LDI R1, @42\n").tokenize();
        } catch (const LexError&) {
            threw = true;
        }

        assert(threw);
    }

    {
        const auto tokens = Lexer("ADD\n").tokenize();

        assert(tokens[0].line == 1);
        assert(tokens[0].column == 1);

        assert(tokens[1].line == 1);
        assert(tokens[1].column == 4);

        assert(tokens[2].line == 2);
        assert(tokens[2].column == 1);
    }

    return 0;
}

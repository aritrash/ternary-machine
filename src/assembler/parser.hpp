#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "assembler/ir.hpp"
#include "assembler/lexer.hpp"

namespace ternary_machine::assembler {

class ParseError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Parser final {
public:
    explicit Parser(const std::vector<Token>& tokens) noexcept : tokens_(tokens) {}

    [[nodiscard]] AssemblyProgram parse() {
        AssemblyProgram program;

        while (!check(TokenKind::EndOfFile)) {
            if (match(TokenKind::Newline))
                continue;

            program.statements.push_back(parse_statement());
        }

        return program;
    }

private:
    [[nodiscard]] Statement parse_statement() {
        if (!check(TokenKind::Identifier))
            throw error("expected statement");

        if (check_next(TokenKind::Colon))
            return parse_label();

        if (check_text("section"))
            return parse_section();

        if (check_text("global"))
            return parse_global();

        return parse_instruction();
    }

    [[nodiscard]] LabelIR parse_label() {
		const Token name = consume(TokenKind::Identifier, "expected label name");
		(void)consume(TokenKind::Colon, "expected ':' after label");
		consume_statement_end();
		return LabelIR{name.text, {name.line, name.column}};
	}

    [[nodiscard]] SectionIR parse_section() {
		const Token directive = consume_identifier("section");
		(void)consume(TokenKind::Dot, "expected '.' after 'section'");
		const Token name = consume(TokenKind::Identifier, "expected section name");
		consume_statement_end();
		return SectionIR{name.text, {directive.line, directive.column}};
	}

    [[nodiscard]] GlobalIR parse_global() {
        const Token directive = consume_identifier("global");
        const Token name = consume(TokenKind::Identifier, "expected global symbol");

        consume_statement_end();

        return GlobalIR{name.text, {directive.line, directive.column}};
    }

    [[nodiscard]] InstructionIR parse_instruction() {
        const Token mnemonic = consume(TokenKind::Identifier, "expected instruction mnemonic");

        InstructionIR instruction;
        instruction.mnemonic = mnemonic.text;
        instruction.location = {mnemonic.line, mnemonic.column};

        if (check(TokenKind::Newline) || check(TokenKind::EndOfFile)) {
            consume_statement_end();
            return instruction;
        }

        instruction.operands.push_back(parse_operand());

        while (match(TokenKind::Comma))
            instruction.operands.push_back(parse_operand());

        consume_statement_end();

        return instruction;
    }

    [[nodiscard]] Operand parse_operand() {
        if (check(TokenKind::Register))
            return RegisterOperand{consume(TokenKind::Register, "expected register").register_index};

        if (check(TokenKind::Number))
            return ImmediateOperand{consume(TokenKind::Number, "expected number").number};

        if (check(TokenKind::Identifier))
            return SymbolOperand{consume(TokenKind::Identifier, "expected identifier").text};

        if (match(TokenKind::LBracket))
            return parse_memory_operand();

        throw error("expected operand");
    }

    [[nodiscard]] MemoryOperand parse_memory_operand() {
		const Token base = consume(TokenKind::Register, "expected base register inside memory operand");

		std::int64_t offset = 0;

		if (match(TokenKind::Plus))
		    offset = consume(TokenKind::Number, "expected numeric offset after '+'").number;

		(void)consume(TokenKind::RBracket, "expected ']' after memory operand");

		return MemoryOperand{base.register_index, offset};
	}

    void consume_statement_end() {
        if (match(TokenKind::Newline))
            return;

        if (check(TokenKind::EndOfFile))
            return;

        throw error("expected end of statement");
    }

    [[nodiscard]] Token consume(TokenKind kind, const char* message) {
        if (!check(kind))
            throw error(message);

        return tokens_[position_++];
    }

    [[nodiscard]] Token consume_identifier(const char* expected) {
        if (!check(TokenKind::Identifier) || tokens_[position_].text != expected)
            throw error(std::string("expected '") + expected + "'");

        return tokens_[position_++];
    }

    bool match(TokenKind kind) {
        if (!check(kind))
            return false;

        ++position_;
        return true;
    }

    [[nodiscard]] bool check(TokenKind kind) const noexcept {
        return position_ < tokens_.size() && tokens_[position_].kind == kind;
    }

    [[nodiscard]] bool check_next(TokenKind kind) const noexcept {
        return position_ + 1 < tokens_.size() && tokens_[position_ + 1].kind == kind;
    }

    [[nodiscard]] bool check_text(const char* text) const noexcept {
        return check(TokenKind::Identifier) && tokens_[position_].text == text;
    }

    [[nodiscard]] ParseError error(const std::string& message) const {
        if (position_ >= tokens_.size())
            return ParseError(message + " at end of input");

        const Token& token = tokens_[position_];

        return ParseError(
            message +
            " at line " + std::to_string(token.line) +
            ", column " + std::to_string(token.column)
        );
    }

    const std::vector<Token>& tokens_;
    std::size_t position_ = 0;
};

}

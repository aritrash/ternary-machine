#pragma once

#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ternary_machine::assembler {

enum class TokenKind {
    Identifier,
    Register,
    Number,
    Colon,
    Comma,
    LBracket,
    RBracket,
    Plus,
    Minus,
    Dot,
    Newline,
    EndOfFile
};

struct Token final {
    TokenKind kind;
    std::string text;
    std::int64_t number = 0;
    std::uint8_t register_index = 0;
    std::size_t line = 1;
    std::size_t column = 1;

    bool operator==(const Token&) const = default;
};

class LexError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Lexer final {
public:
    explicit Lexer(std::string_view source) noexcept : source_(source) {}

    [[nodiscard]] std::vector<Token> tokenize() const {
        std::vector<Token> tokens;
        std::size_t position = 0;
        std::size_t line = 1;
        std::size_t column = 1;

        while (position < source_.size()) {
            const char current = source_[position];

            if (current == ' ' || current == '\t' || current == '\r') {
                ++position;
                ++column;
                continue;
            }

            if (current == ';') {
                while (position < source_.size() && source_[position] != '\n') {
                    ++position;
                    ++column;
                }
                continue;
            }

            if (current == '\n') {
                tokens.push_back({TokenKind::Newline, "\n", 0, 0, line, column});
                ++position;
                ++line;
                column = 1;
                continue;
            }

            const std::size_t token_line = line;
            const std::size_t token_column = column;

            switch (current) {
                case ':':
                    tokens.push_back({TokenKind::Colon, ":", 0, 0, token_line, token_column});
                    ++position;
                    ++column;
                    continue;
                case ',':
                    tokens.push_back({TokenKind::Comma, ",", 0, 0, token_line, token_column});
                    ++position;
                    ++column;
                    continue;
                case '[':
                    tokens.push_back({TokenKind::LBracket, "[", 0, 0, token_line, token_column});
                    ++position;
                    ++column;
                    continue;
                case ']':
                    tokens.push_back({TokenKind::RBracket, "]", 0, 0, token_line, token_column});
                    ++position;
                    ++column;
                    continue;
                case '+':
                    tokens.push_back({TokenKind::Plus, "+", 0, 0, token_line, token_column});
                    ++position;
                    ++column;
                    continue;
                case '-':
                    if (position + 1 < source_.size() && std::isdigit(static_cast<unsigned char>(source_[position + 1]))) {
                        const std::size_t start = position;
                        ++position;
                        ++column;

                        while (position < source_.size() && std::isdigit(static_cast<unsigned char>(source_[position]))) {
                            ++position;
                            ++column;
                        }

                        const std::string text(source_.substr(start, position - start));
                        tokens.push_back({TokenKind::Number, text, parse_number(text, token_line, token_column), 0, token_line, token_column});
                        continue;
                    }

                    tokens.push_back({TokenKind::Minus, "-", 0, 0, token_line, token_column});
                    ++position;
                    ++column;
                    continue;
                case '.':
                    tokens.push_back({TokenKind::Dot, ".", 0, 0, token_line, token_column});
                    ++position;
                    ++column;
                    continue;
                default:
                    break;
            }

            if (std::isdigit(static_cast<unsigned char>(current))) {
                const std::size_t start = position;

                while (position < source_.size() && std::isdigit(static_cast<unsigned char>(source_[position]))) {
                    ++position;
                    ++column;
                }

                const std::string text(source_.substr(start, position - start));
                tokens.push_back({TokenKind::Number, text, parse_number(text, token_line, token_column), 0, token_line, token_column});
                continue;
            }

            if (is_identifier_start(current)) {
                const std::size_t start = position;

                while (position < source_.size() && is_identifier_character(source_[position])) {
                    ++position;
                    ++column;
                }

                const std::string text(source_.substr(start, position - start));

                if (text.size() == 2 && (text[0] == 'R' || text[0] == 'r') && text[1] >= '0' && text[1] <= '8') {
                    tokens.push_back({TokenKind::Register, text, 0, static_cast<std::uint8_t>(text[1] - '0'), token_line, token_column});
                } else {
                    tokens.push_back({TokenKind::Identifier, text, 0, 0, token_line, token_column});
                }

                continue;
            }

            throw LexError(error_message("unexpected character", current, token_line, token_column));
        }

        tokens.push_back({TokenKind::EndOfFile, "", 0, 0, line, column});
        return tokens;
    }

private:
    [[nodiscard]] static constexpr bool is_identifier_start(char value) noexcept {
        const unsigned char c = static_cast<unsigned char>(value);
        return std::isalpha(c) || value == '_';
    }

    [[nodiscard]] static constexpr bool is_identifier_character(char value) noexcept {
        const unsigned char c = static_cast<unsigned char>(value);
        return std::isalnum(c) || value == '_';
    }

    [[nodiscard]] static std::int64_t parse_number(std::string_view text, std::size_t line, std::size_t column) {
        try {
            std::size_t consumed = 0;
            const std::int64_t value = std::stoll(std::string(text), &consumed, 10);

            if (consumed != text.size())
                throw LexError(error_message("invalid integer", text.front(), line, column));

            return value;
        } catch (const std::out_of_range&) {
            throw LexError(error_message("integer literal out of range", text.front(), line, column));
        }
    }

    [[nodiscard]] static std::string error_message(const char* message, char character, std::size_t line, std::size_t column) {
        return std::string(message) + " '" + character + "' at line " + std::to_string(line) + ", column " + std::to_string(column);
    }

    std::string_view source_;
};

}

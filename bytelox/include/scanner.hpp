#pragma once

#include "chunk.hpp"

#include <string>

namespace bytelox {

enum class TokenType {
	// Single-character tokens
	LEFT_PAREN, RIGHT_PAREN,
	LEFT_BRACE, RIGHT_BRACE,
	COMMA, DOT, MINUS, PLUS,
	SEMICOLON, SLASH, STAR,
	// One or two character tokens
	BANG, BANG_EQUAL,
	EQUAL, EQUAL_EQUAL,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,
	// Literals
	IDENTIFIER, STRING, NUMBER,
	// Keywords
	AND, CLASS, ELSE, FALSE,
	FOR, FUN, IF, NIL, OR,
	PRINT, RETURN, SUPER, THIS,
	TRUE, VAR, WHILE,

	ERROR, END_OF_FILE
};

struct Token {
	TokenType type;
	std::string_view lexeme;
	int line;
};

struct Scanner {
	std::string_view src;
	int start = 0;   // start of lexeme
	int current = 0; // current character
	int line = 1;
	
	Scanner(std::string_view src);
	
	Token scan_token();
	Token make_token(TokenType type);
	Token error_token(std::string_view msg);
	char advance();
	[[nodiscard]] char peek() const;
	[[nodiscard]] char peek_next() const;
	bool match(char expected);
	void skip_whitespace();
	[[nodiscard]] bool is_at_end() const;
	Token string();
	Token number();
	Token identifier();
	TokenType identifier_type();
};

}

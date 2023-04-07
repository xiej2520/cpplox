#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "lox.h"
#include "token.h"

class Scanner {
	const std::string src;
	std::vector<Token> tokens;
	size_t start = 0;
	size_t current = 0;
	size_t line = 1;
	std::unordered_map<std::string, TokenType> keywords;
	bool isAtEnd();
	char advance();
	void addToken(TokenType type, LoxObject literal);
	void addToken(TokenType type);
	bool match(char expected);
	char peek();
	char peekNext();
	void read_string();
	void read_number();
	void read_identifier();
	void scanToken();

	public:
	Scanner(std::string src);
	std::vector<Token> scanTokens();
};

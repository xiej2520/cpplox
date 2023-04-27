#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "token.h"

class Scanner {
	const std::string_view src;
	std::vector<Token> tokens;
	int start = 0;
	int current = 0;
	int line = 1;
	std::unordered_map<std::string_view, TokenType> keywords;
	void add_token(TokenType type, TokenLiteral literal);
	void add_token(TokenType type);
	bool is_at_end();
	// returns the current character and advances 1
	char advance();
	// checks if the current char is == expected, if it is then advance 1
	bool match(char expected);
	// returns current char
	char peek();
	// returns next char
	char peek_next();
	void read_string();
	void read_number();
	void read_identifier();
	void scan_token();

	public:
	Scanner(std::string_view src);
	std::vector<Token> scan_tokens();
};

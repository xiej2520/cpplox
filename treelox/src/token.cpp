#include "token.h"

Token::Token(TokenType type, std::string_view lexeme, TokenLiteral literal, int line):
		type(type), lexeme(lexeme), literal(literal), line(line) { }

std::string to_string(const Token &t) {

	return "Token(" + to_string(t.type) + ", " + t.lexeme + ")";
}

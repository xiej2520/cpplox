#include "token.h"

Token::Token(TokenType type, std::string_view lexeme, TokenLiteral literal, int line):
		type(type), lexeme(lexeme), literal(literal), line(line) { }

std::string to_string(TokenLiteral l) {
	if (std::holds_alternative<bool>(l)) {
		return to_string(std::get<bool>(l));
	}
	if (std::holds_alternative<int>(l)) {
		return to_string(std::get<int>(l));
	}
	if (std::holds_alternative<double>(l)) {
		return to_string(std::get<double>(l));
	}
	return "";
}

std::string to_string(const Token &t) {
	return "Token(" + to_string(t.type) + ", " + t.lexeme + ", " + to_string(t.literal) + ")";
}

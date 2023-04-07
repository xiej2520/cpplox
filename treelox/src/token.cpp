#include "token.h"

Token::Token(TokenType t, std::string lex, LoxObject lit, int line):
		type(t), lexeme(lex), literal(lit), line(line) { }

std::string to_string(const LoxObject &o);

std::string Token::repr() const {
	return to_string(type) + " " + lexeme + " " + to_string(literal);
}

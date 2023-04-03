#include "token.h"

Token::Token(TokenType t, std::string lex, LiteralVar lit, int line):
		type(t), lexeme(lex), literal(lit), line(line) { }

std::string Token::repr() {
	return to_string(type) + " " + lexeme + " " +
		std::visit(LiteralToString(), literal);
}

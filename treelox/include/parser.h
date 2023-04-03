#pragma once

#include <vector>
#include "treelox.h"
#include "expr.h"
#include "stmt.h"
#include "token.h"
#include "astprinter.h"

class Parser {
	class ParseError {

	};

	std::vector<Token> tokens;
	int current = 0;
	
	// parses a chain of assignment statements
	Expr assignment();
	// parses a block up to a right brace '}'
	std::vector<Stmt> block();
	// parses an expression
	Expr expression();
	// parses an equality expression '==', '!='
	Expr equality();
	// parses an and expression. Higher precedence than or
	Expr and_expr();
	// parses an or expression
	Expr or_expr();
	// matches the current token against a list of tokens, advances if it is
	bool match(const std::vector<TokenType> types);
	bool match(TokenType type);
	// check if the current token is of TokenType type
	bool check(TokenType type);
	// advances the token counter
	Token advance();
	bool isAtEnd();
	// returns the current token
	Token peek();
	// returns the previous token
	Token previous();
	// parses a comparison expression
	Expr comparison();
	// parses an addition/subtraction expression
	Expr term();
	// parses a multiplication/division expression. Higher precedence than term()
	Expr factor();
	// parses a unary '!' or '-' expression
	Expr unary();
	// parses a call expression
	Expr call();
	// scans forward until the call is completed with a ')'
	Expr finishCall(std::shared_ptr<Expr> callee);
	Expr primary();
	
	Stmt statement();
	Stmt ifStatement();
	Stmt whileStatement();
	Stmt forStatement();
	Stmt printStatement();
	Stmt expressionStatement();
	Stmt declaration();
	Stmt varDeclaration();

	// expects the current token to be of TokenType type, otherwise error
	Token consume(TokenType type, std::string message);
	ParseError error(Token token, std::string message);
	void synchronize();

public:
	Parser(const std::vector<Token> &tokens);
	std::vector<Stmt> parse();
};

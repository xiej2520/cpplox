#pragma once

#include <span>
#include <vector>

#include "expr.h"
#include "stmt.h"
#include "token.h"

class Parser {
	class ParseError { };

	std::vector<Token> tokens;
	int current = 0;

	// matches the current token against a list of tokens, advances if it is
	bool match(const std::vector<TokenType> &types);
	bool match(TokenType type);
	// check if the current token is of TokenType type
	bool check(TokenType type);
	// advances the token counter
	Token advance();
	bool is_at_end();
	// returns the current token
	Token peek();
	// returns the previous token
	Token previous();

	// expects the current token to be of TokenType type, otherwise error
	Token consume(TokenType type, std::string_view message);
	ParseError error(const Token &token, std::string_view message);
	void synchronize();

	
	// parses an expression
	Expr expression();
	// parses a chain of assignment statements
	Expr assignment();
	// parses an or expression
	Expr or_expr();
	// parses an and expression. Higher precedence than or
	Expr and_expr();
	// parses an equality expression '==', '!='
	Expr equality();
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
	Expr finish_call(std::unique_ptr<Expr> callee);
	Expr primary();
	
	Stmt statement();
	// parses a block up to a right brace '}'
	// returns a vector because this is reused for function()
	std::vector<Stmt> block();
	Stmt if_statement();
	Stmt while_statement();
	Stmt for_statement();
	Stmt print_statement();
	Stmt return_statement();
	Stmt expression_statement();
	Stmt function(std::string kind);
	Stmt declaration();
	Stmt var_declaration();

public:
	Parser(std::span<Token> tokens);
	std::vector<Stmt> parse();
};

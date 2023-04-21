#include "parser.h"

#include <ranges>
using enum TokenType;
using std::make_shared;
using std::string;
using std::vector;

Parser::Parser(const vector<Token> &tokens): tokens(tokens) {}

Expr Parser::assignment() {
	Expr expr(or_expr());

	if (match(EQUAL)) {
		Token equals(previous());
		Expr value(assignment());
		
		if (std::holds_alternative<Variable>(expr)) {
			Token name = std::get<Variable>(expr).name;
			return Assign(name, make_shared<Expr>(value));
		}
		Lox::error(equals, "Invalid assignment target.");
	}
	return expr;
}

vector<Stmt> Parser::block() {
	vector<Stmt> statements;
	while (!check(RIGHT_BRACE) && !isAtEnd()) {
		statements.push_back(declaration());
	}
	consume(RIGHT_BRACE, "Expect '}' after block.");
	return statements;
}

Expr Parser::expression() {
	return assignment();
}

Expr Parser::equality() {
	Expr expr = comparison();
	while (match({BANG_EQUAL, EQUAL_EQUAL})) {
		Token op = previous();
		auto right = make_shared<Expr>(comparison());
		expr.emplace<Binary>(make_shared<Expr>(expr), op, right);
	}
	return expr;
}

Expr Parser::and_expr() {
	Expr expr = equality();
	while (match(AND)) {
		Token op = previous();
		auto right = make_shared<Expr>(equality());
		expr.emplace<Logical>(make_shared<Expr>(expr), op, right);
	}
	return expr;
}

Expr Parser::or_expr() {
	Expr expr = and_expr();
	while (match(OR)) {
		Token op = previous();
		auto right = make_shared<Expr>(and_expr());
		expr.emplace<Logical>(make_shared<Expr>(expr), op, right);
	}
	return expr;
}

bool Parser::match(vector<TokenType> types) {
	for (TokenType type : types) {
		if (check(type)) {
			advance();
			return true;
		}
	}
	return false;
}

bool Parser::match(TokenType type) {
	if (check(type)) {
		advance();
		return true;
	}
	return false;
}

bool Parser::check(TokenType type) {
	if (isAtEnd()) return false;
	return peek().type == type;
}

Token Parser::advance() {
	if (!isAtEnd()) current++;
	return previous();
}

bool Parser::isAtEnd() {
	return peek().type == END_OF_FILE;
}

Token Parser::peek() {
	return tokens[current];
}

Token Parser::previous() {
	return tokens[current - 1];
}

Expr Parser::comparison() {
	Expr expr = term();
	while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
		Token op = previous();
		Expr right = term();
		expr.emplace<Binary>(make_shared<Expr>(expr), op, make_shared<Expr>(right));
	}
	return expr;
}

Expr Parser::term() {
	Expr expr = factor();
	while (match({MINUS, PLUS})) {
		Token op = previous();
		Expr right = factor();
		expr.emplace<Binary>(make_shared<Expr>(expr), op, make_shared<Expr>(right));
	}
	return expr;
}

Expr Parser::factor() {
	Expr expr = unary();
	while (match({SLASH, STAR})) {
		Token op = previous();
		Expr right = unary();
		expr.emplace<Binary>(make_shared<Expr>(expr), op, make_shared<Expr>(right));
	}
	return expr;
}

Expr Parser::unary() {
	if (match({BANG, MINUS})) {
		Token op = previous();
		Expr right = unary();
		return Unary(op, make_shared<Expr>(right));
	}
	return call();
}

Expr Parser::call() {
	auto expr = make_shared<Expr>(primary());
	while (true) {
		if (match(LEFT_PAREN)) {
			return finishCall(expr);
		}
		Expr res = *expr;
		return res;
	}
}

Expr Parser::finishCall(std::shared_ptr<Expr> callee) {
	vector<Expr> arguments;
	if (!check(RIGHT_PAREN)) {
		do {
			// doesn't throw error, just reports it
			if (arguments.size() >= 255) {
				// error(peek(), "Can't have more than 255 arguments.");
			}
			arguments.push_back(expression());
		} while (match(COMMA));
	}
	Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");
	return Call(callee, paren, arguments);
}

Expr Parser::primary() {
	if (match(FALSE)) return Literal(false);
	if (match(TRUE)) return Literal(true);
	if (match(NIL)) return Literal(std::monostate{});
	if (match({NUMBER, STRING})) {
		return Literal(previous().literal);
	}
	if (match(LEFT_PAREN)) {
		Expr expr(expression());
		consume(RIGHT_PAREN, "Expect ')' after expression.");
		return Grouping(make_shared<Expr>(expr));
	}
	if (match(IDENTIFIER)) {
		return Variable(previous());
	}
	throw error(peek(), "Expect expression.");
}

Token Parser::consume(TokenType type, string message) {
	if (check(type)) return advance();

	// find a way to avoid exceptions
	throw error(peek(), message);
}

Parser::ParseError Parser::error(Token token, string message) {
	Lox::error(token, message);
	return ParseError{};
}

void Parser::synchronize() {
	advance();
	while (!isAtEnd()) {
		if (previous().type == SEMICOLON) return;
		switch (peek().type) {
			case CLASS:
			case FUN:
			case VAR:
			case FOR:
			case IF:
			case WHILE:
			case PRINT:
			case RETURN:
				return;
			default: // ???
				return;
		}
		advance();
	}
}

vector<Stmt> Parser::parse() {
	vector<Stmt> statements;
	while (!isAtEnd()) {
		statements.push_back(declaration());
	}
	return statements;
}

Stmt Parser::statement() {
	if (match(FOR)) return forStatement();
	if (match(IF)) return ifStatement();
	if (match(PRINT)) return printStatement();
	if (match(RETURN)) return returnStatement();
	if (match(WHILE)) return whileStatement();
	if (match(LEFT_BRACE)) return Block(block());
	return expressionStatement();
}

Stmt Parser::ifStatement() {
	consume(LEFT_PAREN, "Expect '(' after 'if'.");
	Expr condition = expression();
	consume(RIGHT_PAREN, "Expect ')' after if condition.");
	
	auto thenBranch = make_shared<Stmt>(statement());
	auto elseBranch = make_shared<Stmt>(std::monostate{});
	if (match(ELSE)) {
		elseBranch = make_shared<Stmt>(statement());
	}
	return If(condition, thenBranch, elseBranch);
}

Stmt Parser::whileStatement() {
	consume(LEFT_PAREN, "Expect '(' after 'while'.");
	Expr condition = expression();
	consume(RIGHT_PAREN, "Expect ')' after condition.");
	auto body = make_shared<Stmt>(statement());
	return While(condition, body);
}

Stmt Parser::forStatement() {
	consume(LEFT_PAREN, "Expect '(' after 'for'.");
	Stmt initializer = match(SEMICOLON) ? std::monostate{} : match(VAR) ?
		varDeclaration() : expressionStatement();
	
	Expr condition = check(SEMICOLON) ? Literal(true) : expression();
	consume(SEMICOLON, "Expect ';' after loop condition.");
	
	Expr increment = check(RIGHT_PAREN) ? std::monostate{} : expression();
	consume(RIGHT_PAREN, "Expect ')' after for clauses.");
	
	// has both initializer and increment
	if (!std::holds_alternative<std::monostate>(initializer) &&
		!std::holds_alternative<std::monostate>(increment)) {
		return Block({initializer, While(condition, make_shared<Stmt>(
			Block({statement(), Expression(increment)}))
		)});
	}
	// initializer is empty
	if (!std::holds_alternative<std::monostate>(increment)) {
		return While(condition, make_shared<Stmt>(Block({statement(), Expression(increment)})));
	}
	// increment is empty
	if (!std::holds_alternative<std::monostate>(initializer)) {
		return Block({initializer, While(condition, make_shared<Stmt>(statement()))});
	}
	// infinite loop
	return While(condition, make_shared<Stmt>(Block({statement()})));
}

Stmt Parser::printStatement() {
	Expr value = expression();
	consume(SEMICOLON, "Expect ';' after value.");
	return Print(value);
}

Stmt Parser::returnStatement() {
	Token keyword = previous();
	Expr value = check(SEMICOLON) ? std::monostate{} : expression();
	consume(SEMICOLON, "Expect ';' after return value.");
	return Return(keyword, value);
}

Stmt Parser::expressionStatement() {
	Expr expr = expression();
	consume(SEMICOLON, "Expect ';' after expression.");
	return Expression(expr);
}

Function Parser::function(string kind) {
	Token name = consume(IDENTIFIER, "Expect " + kind + " name.");
	consume(LEFT_PAREN, "Expect '(' after " + kind + " name.");
	vector<Token> parameters;
	if (!check(RIGHT_PAREN)) {
		do {
			// allow any number of parameters
			parameters.push_back(
				consume(IDENTIFIER, "Expect parameter name.")
			);
		} while (match(COMMA));
	}
	consume(RIGHT_PAREN, "Expect ')' after parameters.");
	consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
	vector<Stmt> body = block();
	return Function(name, parameters, body);
}

Stmt Parser::declaration() {
	try {
		if (match(FUN)) return function("function");
		if (match(VAR)) return varDeclaration();
		return statement();
	}
	catch (ParseError &err) {
		synchronize();
		return std::monostate{};
	}
}

Stmt Parser::varDeclaration() {
	Token name = consume(IDENTIFIER, "Expect variable name.");
	// default copy assignment operator deleted (const members), use pointer?
	std::optional<Expr> initializer = std::nullopt;
	if (match(EQUAL)) {
		initializer.emplace(expression());
	}
	consume(SEMICOLON, "Expect ';' after variable declaration.");
	return Var(name, initializer);
}

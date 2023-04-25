#include "parser.h"

#include "lox.h"

using enum TokenType;
using std::make_unique;
using std::string;
using std::string_view;
using std::span;
using std::vector;

constexpr size_t MAX_ARGS = 255;

Parser::Parser(span<Token> tokens): tokens(tokens.begin(), tokens.end()) {}

bool Parser::match(const vector<TokenType> &types) {
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
	if (is_at_end()) {
		return false;
	}
	return peek().type == type;
}

Token Parser::advance() {
	if (!is_at_end()) {
		current++;
	}
	return previous();
}

bool Parser::is_at_end() {
	return peek().type == END_OF_FILE;
}

Token Parser::peek() {
	return tokens[current];
}

Token Parser::previous() {
	return tokens[current - 1];
}

Token Parser::consume(TokenType type, string_view message) {
	if (check(type)) {
		return advance();
	}
	// find a way to avoid exceptions
	throw error(peek(), message);
}

Parser::ParseError Parser::error(const Token &token, string_view message) {
	Lox::error(token, message);
	return ParseError{};
}

void Parser::synchronize() {
	advance();
	while (!is_at_end()) {
		if (previous().type == SEMICOLON) {
			return;
		}
		switch (peek().type) {
			/*
			case CLASS:
			case FUN:
			case VAR:
			case FOR:
			case IF:
			case WHILE:
			case PRINT:
			case RETURN:
				return;
			*/
			default: // ???
				return;
		}
		advance();
	}
}


Expr Parser::expression() {
	return assignment();
}

Expr Parser::assignment() {
	Expr expr(or_expr());

	if (match(EQUAL)) {
		Token equals(previous());
		Expr value(assignment());
		
		if (holds_alternative<Variable>(expr)) {
			Token name = get<Variable>(expr).name;
			return Assign(name, make_unique<Expr>(std::move(value)));
		}
		if (holds_alternative<Get>(expr)) { // convert Get to Set if it is on the left of =
			Get &get_expr(get<Get>(expr));
			return Set(std::move(get_expr.object), get_expr.name, make_unique<Expr>(std::move(value)));
		}
		Lox::error(equals, "Invalid assignment target.");
	}
	return expr;
}

Expr Parser::or_expr() {
	Expr expr = and_expr();
	while (match(OR)) {
		Token op = previous();
		auto right = make_unique<Expr>(and_expr());
		expr.emplace<Logical>(make_unique<Expr>(std::move(expr)), op, std::move(right));
	}
	return expr;
}

Expr Parser::and_expr() {
	Expr expr = equality();
	while (match(AND)) {
		Token op = previous();
		auto right = make_unique<Expr>(equality());
		expr.emplace<Logical>(make_unique<Expr>(std::move(expr)), op, std::move(right));
	}
	return expr;
}

Expr Parser::equality() {
	Expr expr = comparison();
	while (match({BANG_EQUAL, EQUAL_EQUAL})) {
		Token op = previous();
		auto right = make_unique<Expr>(comparison());
		expr.emplace<Binary>(make_unique<Expr>(std::move(expr)), op, std::move(right));
	}
	return expr;
}

Expr Parser::comparison() {
	Expr expr = term();
	while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
		Token op = previous();
		Expr right = term();
		expr.emplace<Binary>(make_unique<Expr>(std::move(expr)), op, make_unique<Expr>(std::move(right)));
	}
	return expr;
}

Expr Parser::term() {
	Expr expr = factor();
	while (match({MINUS, PLUS})) {
		Token op = previous();
		Expr right = factor();
		expr.emplace<Binary>(make_unique<Expr>(std::move(expr)), op, make_unique<Expr>(std::move(right)));
	}
	return expr;
}

Expr Parser::factor() {
	Expr expr = unary();
	while (match({SLASH, STAR})) {
		Token op = previous();
		Expr right = unary();
		expr.emplace<Binary>(make_unique<Expr>(std::move(expr)), op, make_unique<Expr>(std::move(right)));
	}
	return expr;
}

Expr Parser::unary() {
	if (match({BANG, MINUS})) {
		Token op = previous();
		Expr right = unary();
		return Unary(op, make_unique<Expr>(std::move(right)));
	}
	return call();
}

Expr Parser::call() {
	auto expr = make_unique<Expr>(primary());
	while (true) { // allow chained call Expr
		if (match(LEFT_PAREN)) {
			expr = make_unique<Expr>(finish_call(std::move(expr)));
		}
		else if (match(DOT)) {
			Token name = consume(IDENTIFIER, "Expect property name after '.'.");
			expr = make_unique<Expr>(Get(std::move(expr), name));
		}
		else {
			break;
		}
	}
	Expr res(std::move(*expr));
	return res;
}

Expr Parser::finish_call(std::unique_ptr<Expr> callee) {
	vector<Expr> arguments;
	if (!check(RIGHT_PAREN)) {
		do {
			// doesn't throw error, just reports it
			if (arguments.size() >= MAX_ARGS) {
				// error(peek(), "Can't have more than MAX_ARGS arguments.");
			}
			arguments.push_back(expression());
		} while (match(COMMA));
	}
	Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");
	return Call(std::move(callee), paren, std::move(arguments));
}

struct TokenLiteralConversion {
	Token &t;
	TokenLiteralConversion(Token &t): t(t) {}
	Literal operator()(std::monostate){ return {t.lexeme.substr(1, t.lexeme.size() - 2)}; } // get rid of quotes
	Literal operator()(bool b) { return {b}; }
	Literal operator()(int i) { return {i}; }
	Literal operator()(double d) { return {d}; }
};

Expr Parser::primary() {
	if (match(FALSE)) {
		return Literal(false);
	}
	if (match(TRUE)) {
		return Literal(true);
	}
	if (match(NIL)) {
		return Literal(std::monostate{});
	}
	if (match({NUMBER, STRING})) {
		Token t = previous();
		return std::visit(TokenLiteralConversion(t), t.literal);
	}
	if (match(LEFT_PAREN)) {
		Expr expr(expression());
		consume(RIGHT_PAREN, "Expect ')' after expression.");
		return Grouping(make_unique<Expr>(std::move(expr)));
	}
	if (match(IDENTIFIER)) {
		return Variable(previous());
	}
	throw error(peek(), "Expect expression.");
}
vector<Stmt> Parser::parse() {
	vector<Stmt> statements;
	while (!is_at_end()) {
		statements.push_back(declaration());
	}
	return statements;
}

Stmt Parser::statement() {
	if (match(FOR)) return for_statement();
	if (match(IF)) return if_statement();
	if (match(PRINT)) return print_statement();
	if (match(RETURN)) return return_statement();
	if (match(WHILE)) return while_statement();
	if (match(LEFT_BRACE)) return Block(block());
	return expression_statement();
}

vector<Stmt> Parser::block() {
	vector<Stmt> statements;
	while (!check(RIGHT_BRACE) && !is_at_end()) {
		statements.emplace_back(declaration());
	}
	consume(RIGHT_BRACE, "Expect '}' after block.");
	return statements;
}

Stmt Parser::if_statement() {
	consume(LEFT_PAREN, "Expect '(' after 'if'.");
	Expr condition = expression();
	consume(RIGHT_PAREN, "Expect ')' after if condition.");
	
	auto thenBranch = make_unique<Stmt>(statement());
	auto elseBranch = make_unique<Stmt>(std::monostate{});
	if (match(ELSE)) {
		elseBranch = make_unique<Stmt>(statement());
	}
	return If(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

Stmt Parser::while_statement() {
	consume(LEFT_PAREN, "Expect '(' after 'while'.");
	Expr condition = expression();
	consume(RIGHT_PAREN, "Expect ')' after condition.");
	auto body = make_unique<Stmt>(statement());
	return While(std::move(condition), std::move(body));
}

Stmt Parser::for_statement() {
	consume(LEFT_PAREN, "Expect '(' after 'for'.");
	Stmt initializer = match(SEMICOLON) ? std::monostate{} : match(VAR) ?
		var_declaration() : expression_statement();
	
	Expr condition = check(SEMICOLON) ? Literal(true) : expression();
	consume(SEMICOLON, "Expect ';' after loop condition.");
	
	Expr increment = check(RIGHT_PAREN) ? std::monostate{} : expression();
	consume(RIGHT_PAREN, "Expect ')' after for clauses.");
	
	// has both initializer and increment
	if (!std::holds_alternative<std::monostate>(initializer) &&
		!std::holds_alternative<std::monostate>(increment)) {
		vector<Stmt> loop_body;
		loop_body.push_back(statement());
		loop_body.push_back(Expression(std::move(increment)));

		vector<Stmt> for_block;
		for_block.push_back(std::move(initializer));
		for_block.push_back(While(std::move(condition), make_unique<Stmt>(std::move(loop_body))));
		return Block(std::move(for_block));
		/*
		return Block({initializer, While(std::move(condition), make_unique<Stmt>(
			Block({statement(), Expression(std::move(increment))}))
		)});
		*/
	}
	// initializer is empty
	if (!std::holds_alternative<std::monostate>(increment)) {
		vector<Stmt> loop_body;
		loop_body.push_back(statement());
		loop_body.push_back(Expression(std::move(increment)));
		
		return While(std::move(condition), make_unique<Stmt>(std::move(loop_body)));
	}
	// increment is empty
	if (!std::holds_alternative<std::monostate>(initializer)) {
		vector<Stmt> for_block;
		for_block.push_back(std::move(initializer));
		for_block.push_back(While(std::move(condition), make_unique<Stmt>(statement())));
		return Block(std::move(for_block));
	}
	// infinite loop
	vector<Stmt> loop_body;
	return While(std::move(condition), make_unique<Stmt>(statement()));
}

Stmt Parser::print_statement() {
	Expr value = expression();
	consume(SEMICOLON, "Expect ';' after value.");
	return Print(std::move(value));
}

Stmt Parser::return_statement() {
	Token keyword = previous();
	Expr value = check(SEMICOLON) ? std::monostate{} : expression();
	consume(SEMICOLON, "Expect ';' after return value.");
	return Return(keyword, std::move(value));
}

Stmt Parser::expression_statement() {
	Expr expr = expression();
	consume(SEMICOLON, "Expect ';' after expression.");
	return Expression(std::move(expr));
}

Stmt Parser::function(const string &kind) {
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
	return Function(name, parameters, block());
}

Stmt Parser::declaration() {
	try {
		if (match(CLASS)) {
			return class_declaration();
		}
		if (match(FUN)) {
			return function("function");
		}
		if (match(VAR)) {
			return var_declaration();
		}
		return statement();
	}
	catch (ParseError &err) {
		synchronize();
		return std::monostate{};
	}
}

Stmt Parser::class_declaration() {
	Token name = consume(IDENTIFIER, "Expect class name.");
	consume(LEFT_BRACE, "Expect '{' before class body.");
	
	vector<Function> methods;
	while (!check(RIGHT_BRACE) && !is_at_end()) {
		methods.emplace_back(std::move(get<Function>(function("method"))));
	}
	
	consume(RIGHT_BRACE, "Expect '}' after class body.");
	return Class(name, std::move(methods));
}

Stmt Parser::var_declaration() {
	Token name = consume(IDENTIFIER, "Expect variable name.");
	// default copy assignment operator deleted (const members), use pointer?
	std::optional<Expr> initializer = std::nullopt;
	if (match(EQUAL)) {
		initializer.emplace(expression());
	}
	consume(SEMICOLON, "Expect ';' after variable declaration.");
	return Var(name, std::move(initializer));
}

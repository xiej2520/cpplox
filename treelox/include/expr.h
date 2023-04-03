#pragma once

#include "token.h"
#include <memory>
#include <variant>
#include <vector>

struct Assign;
struct Binary;
struct Call;
struct Grouping;
struct Literal;
struct Logical;
struct Unary;
struct Variable;

using Expr = std::variant
<
	std::monostate,
	Assign,
	Binary,
	Call,
	Grouping,
	Literal,
	Logical,
	Unary,
	Variable
>;

/*
 * NEED shared_ptr FOR INDIRECTION!
 * const members deletes default assignment operator
 */
struct Assign {
	const Token name;
	const std::shared_ptr<Expr> value;
	Assign(Token name, std::shared_ptr<Expr> value): name(name), value(value) {}
};

struct Binary {
	const std::shared_ptr<Expr> left;
	const Token op;
	const std::shared_ptr<Expr> right;
	Binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right): left(left), op(op), right(right) {}
};

struct Grouping {
	const std::shared_ptr<Expr> expression;
	Grouping(std::shared_ptr<Expr> expression): expression(expression) {}
};

struct Literal {
	const LiteralVar value;
	Literal(LiteralVar value): value(value) {}
};

struct Logical {
	const std::shared_ptr<Expr> left;
	const Token op;
	const std::shared_ptr<Expr> right;
	Logical(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right): left(left), op(op), right(right) {}
};

struct Unary {
	const Token op;
	const std::shared_ptr<Expr> right;
	Unary(Token op, std::shared_ptr<Expr> right): op(op), right(right) {}
};

struct Variable {
	const Token name;
	Variable(Token name): name(name) {}
};

struct Call {
	const std::shared_ptr<Expr> callee;
	const Token paren;
	const std::vector<Expr> arguments;
	Call(std::shared_ptr<Expr> callee, Token paren, std::vector<Expr> arguments): callee(callee), paren(paren), arguments(arguments) {}
};

struct TreeLoxFunction {

};

struct TreeLoxClass {

};

using ExprEvalRes = std::variant<LiteralVar, TreeLoxFunction, TreeLoxClass>;

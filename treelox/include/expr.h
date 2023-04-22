#pragma once

#include "lox_object.h"
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
 * Need indirection
 * std::unique_ptr can't be marked const, otherwise it can't be moved from in the constructor
 * Do not assign Exprs
 */

struct Assign {
	const Token name;
	std::unique_ptr<Expr> value;
	Assign(Token name, std::unique_ptr<Expr> value);
};

struct Binary {
	std::unique_ptr<Expr> left;
	const Token op;
	std::unique_ptr<Expr> right;
	Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right);
};

struct Call {
	std::unique_ptr<Expr> callee;
	const Token paren;
	std::vector<Expr> arguments;
	Call(std::unique_ptr<Expr> callee, Token paren, std::vector<Expr> arguments);
};

struct Grouping {
	std::unique_ptr<Expr> expression;
	Grouping(std::unique_ptr<Expr> expression);
};

struct Literal {
	const LoxObject value;
	Literal(LoxObject value);
};

struct Logical {
	std::unique_ptr<Expr> left;
	const Token op;
	std::unique_ptr<Expr> right;
	Logical(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right);
};

struct Unary {
	const Token op;
	std::unique_ptr<Expr> right;
	Unary(Token op, std::unique_ptr<Expr> right);
};

struct Variable {
	const Token name;
	Variable(Token name);
};

using LocalsKeys = std::variant<Assign, Variable>;

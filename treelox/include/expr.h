#pragma once

#include "lox_object.h"
#include "token.h"
#include <memory>
#include <variant>
#include <vector>

struct Assign;
struct Binary;
struct Call;
struct Get;
struct Grouping;
struct Literal;
struct Logical;
struct Set;
struct Super;
struct This;
struct Unary;
struct Variable;

using Expr = std::variant
<
	std::monostate,
	Assign,
	Binary,
	Call,
	Get,
	Grouping,
	Literal,
	Logical,
	Set,
	Super,
	This,
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
	int depth;
	LoxObject *var_ptr = nullptr;
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

struct Get {
	std::unique_ptr<Expr> object;
	const Token name;
	Get(std::unique_ptr<Expr> object, Token name);
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

struct Set {
	std::unique_ptr<Expr> object;
	const Token name;
	std::unique_ptr<Expr> value;
	Set(std::unique_ptr<Expr> object, Token name, std::unique_ptr<Expr> value);
};

struct Super {
	const Token keyword;
	const Token method;
	int depth;
	LoxObject *var_ptr = nullptr;
	Super(Token keyword, Token method);
};

struct This {
	const Token keyword;
	int depth;
	LoxObject *var_ptr = nullptr;
	This(Token keyword);
};

struct Unary {
	const Token op;
	std::unique_ptr<Expr> right;
	Unary(Token op, std::unique_ptr<Expr> right);
};

struct Variable {
	const Token name;
	int depth;
	LoxObject *var_ptr = nullptr;
	Variable(Token name);
};

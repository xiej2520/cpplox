#pragma once

#include "expr.h"
#include <optional>
#include <memory>
#include <variant>
#include <vector>

struct Block;
struct Expression;
struct Function;
struct If;
struct Print;
struct Return;
struct Var;
struct While;

using Stmt = std::variant
<
	std::monostate,
	Block,
	Expression,
	Function,
	If,
	Print,
	Return,
	Var,
	While
>;

struct Block {
	std::vector<Stmt> statements;
	Block(std::vector<Stmt> statements);
};


struct Expression {
	Expr expression;
	Expression(Expr expression);
};

struct Function {
	const Token name;
	std::vector<Token> params;
	std::vector<Stmt> body;
	Function(Token name, std::vector<Token> params, std::vector<Stmt> body);
};

struct If {
	Expr condition;
	std::unique_ptr<Stmt> then_branch;
	std::unique_ptr<Stmt> else_branch;
	If(Expr condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch);
};

struct Print {
	Expr expression;
	Print(Expr expression);
};

struct Return {
	const Token keyword;
	Expr value;
	Return(Token keyword, Expr value);
};

struct Var {
	const Token name;
	std::optional<Expr> initializer;
	Var(Token name, std::optional<Expr> initializer);
};

struct While {
	Expr condition;
	std::unique_ptr<Stmt> body;
	While(Expr condition, std::unique_ptr<Stmt> body);
};

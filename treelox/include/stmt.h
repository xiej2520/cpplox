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
	Var,
	While
>;

struct Expression {
	const Expr expression;
	Expression(Expr expression);
};

struct If {
	const Expr condition;
	const std::shared_ptr<Stmt> thenBranch;
	const std::shared_ptr<Stmt> elseBranch;
	If(Expr condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch);
};

struct Print {
	const Expr expression;
	Print(Expr expression);
};

struct Var {
	const Token name;
	const std::optional<Expr> initializer;
	Var(Token name, std::optional<Expr> initializer);
};

struct While {
	const Expr condition;
	const std::shared_ptr<Stmt> body;
	While(Expr condition, std::shared_ptr<Stmt> body);
};

struct Function {
	const Token name;
	const std::vector<Token> params;
	const std::vector<Stmt> body;
	Function(Token name, std::vector<Token> params, std::vector<Stmt> body);
};

struct Block {
	const std::vector<Stmt> statements;
	Block(std::vector<Stmt> statements);
};

#pragma once

#include "expr.h"
#include <optional>
#include <memory>
#include <variant>
#include <vector>

struct Block;
struct Expression;
struct If;
struct Print;
struct Var;
struct While;

using Stmt = std::variant
<
	std::monostate,
	Block,
	Expression,
	If,
	Print,
	Var,
	While
>;

struct Expression {
	const Expr expression;
	Expression(Expr expression): expression(expression) {}
};

struct If {
	const Expr condition;
	const std::shared_ptr<Stmt> thenBranch;
	const std::shared_ptr<Stmt> elseBranch;
	If(Expr condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch):
		condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
};

struct Print {
	const Expr expression;
	Print(Expr expression): expression(expression) {}
};

struct Var {
	const Token name;
	const std::optional<Expr> initializer;
	Var(Token name, std::optional<Expr> initializer): name(name), initializer(initializer) {}
};

struct While {
	const Expr condition;
	const std::shared_ptr<Stmt> body;
	While(Expr condition, std::shared_ptr<Stmt> body): condition(condition), body(body) {}
};

// Block comes last to shut up clang
struct Block {
	const std::vector<Stmt> statements;
	Block(std::vector<Stmt> statements): statements(statements) {}
};

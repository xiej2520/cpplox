#include "stmt.h"

using std::optional;
using std::unique_ptr;
using std::vector;

static_assert(std::is_move_constructible_v<Stmt>, "not move constructible");
static_assert(std::is_move_constructible_v<Block>, "not move constructible");
static_assert(std::is_move_constructible_v<Expression>, "not move constructible");
static_assert(std::is_move_constructible_v<Function>, "not move constructible");
static_assert(std::is_move_constructible_v<If>, "not move constructible");
static_assert(std::is_move_constructible_v<Print>, "not move constructible");
static_assert(std::is_move_constructible_v<Return>, "not move constructible");
static_assert(std::is_move_constructible_v<Var>, "not move constructible");
static_assert(std::is_move_constructible_v<While>, "not move constructible");

Block::Block(vector<Stmt> statements): statements(std::move(statements)) {}

Class::Class(Token name, optional<Variable> superclass, vector<Function> methods):
	name(std::move(name)), superclass(std::move(superclass)), methods(std::move(methods)) {}

Expression::Expression(Expr expression): expression(std::move(expression)) {}

Function::Function(Token name, vector<Token> params, vector<Stmt> body):
	name(std::move(name)), params(std::move(params)), body(std::move(body)) {}

If::If(Expr condition, unique_ptr<Stmt> then_branch, unique_ptr<Stmt> else_branch):
	condition(std::move(condition)), then_branch(std::move(then_branch)), else_branch(std::move(else_branch)) {}

Print::Print(Expr expression): expression(std::move(expression)) {}

Return::Return(Token keyword, Expr value):
	keyword(std::move(keyword)), value(std::move(value)) {}

Var::Var(Token name, optional<Expr> initializer):
	name(std::move(name)), initializer(std::move(initializer)) {}

While::While(Expr condition, unique_ptr<Stmt> body):
	condition(std::move(condition)), body(std::move(body)) {}

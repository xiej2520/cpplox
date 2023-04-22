#include "stmt.h"

using std::unique_ptr;
using std::vector;

static_assert(std::is_move_constructible<Stmt>::value, "not move constructible");
static_assert(std::is_move_constructible<Block>::value, "not move constructible");
static_assert(std::is_move_constructible<Expression>::value, "not move constructible");
static_assert(std::is_move_constructible<Function>::value, "not move constructible");
static_assert(std::is_move_constructible<If>::value, "not move constructible");
static_assert(std::is_move_constructible<Print>::value, "not move constructible");
static_assert(std::is_move_constructible<Return>::value, "not move constructible");
static_assert(std::is_move_constructible<Var>::value, "not move constructible");
static_assert(std::is_move_constructible<While>::value, "not move constructible");

Block::Block(vector<Stmt> statements): statements(std::move(statements)) {}

Expression::Expression(Expr expression): expression(std::move(expression)) {}

Function::Function(Token name, vector<Token> params, vector<Stmt> body):
	name(std::move(name)), params(std::move(params)), body(std::move(body)) {}

If::If(Expr condition, unique_ptr<Stmt> then_branch, unique_ptr<Stmt> else_branch):
	condition(std::move(condition)), then_branch(std::move(then_branch)), else_branch(std::move(else_branch)) {}

Print::Print(Expr expression): expression(std::move(expression)) {}

Return::Return(Token keyword, Expr value):
	keyword(std::move(keyword)), value(std::move(value)) {}

Var::Var(Token name, std::optional<Expr> initializer):
	name(std::move(name)), initializer(std::move(initializer)) {}

While::While(Expr condition, unique_ptr<Stmt> body):
	condition(std::move(condition)), body(std::move(body)) {}

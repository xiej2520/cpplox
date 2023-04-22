#include "expr.h"

using std::unique_ptr;

static_assert(std::is_move_constructible<Expr>::value, "not move constructible");

Assign::Assign(Token name, unique_ptr<Expr> value):
	name(std::move(name)), value(std::move(value)) {}

Binary::Binary(unique_ptr<Expr> left, Token op, unique_ptr<Expr> right):
	left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

Call::Call(unique_ptr<Expr> callee, Token paren, std::vector<Expr> arguments):
	callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(arguments)) {}

Grouping::Grouping(unique_ptr<Expr> expression):
	expression(std::move(expression)) {}

Literal::Literal(LoxObject value): value(value) {}

Logical::Logical(unique_ptr<Expr> left, Token op, unique_ptr<Expr> right):
	left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

Unary::Unary(Token op, unique_ptr<Expr> right): op(std::move(op)), right(std::move(right)) {}

Variable::Variable(Token name): name(std::move(name)) {}

#include "expr.h"

using std::move;
using std::unique_ptr;

static_assert(std::is_move_constructible<Expr>::value, "not move constructible");

Assign::Assign(Token name, unique_ptr<Expr> value):
	name(move(name)), value(move(value)) {}

Binary::Binary(unique_ptr<Expr> left, Token op, unique_ptr<Expr> right):
	left(move(left)), op(move(op)), right(move(right)) {}

Call::Call(unique_ptr<Expr> callee, Token paren, std::vector<Expr> arguments):
	callee(move(callee)), paren(move(paren)), arguments(move(arguments)) {}

Get::Get(unique_ptr<Expr> object, Token name): object(move(object)), name(move(name)) {}

Grouping::Grouping(unique_ptr<Expr> expression):
	expression(move(expression)) {}

Literal::Literal(LoxObject value): value(move(value)) {}

Logical::Logical(unique_ptr<Expr> left, Token op, unique_ptr<Expr> right):
	left(move(left)), op(move(op)), right(move(right)) {}

Set::Set(unique_ptr<Expr> object, Token name, unique_ptr<Expr> value):
	object(move(object)), name(move(name)), value(move(value)) {};

Super::Super(Token keyword, Token method): keyword(move(keyword)), method(move(method)) {}

This::This(Token keyword): keyword(move(keyword)) {}

Unary::Unary(Token op, unique_ptr<Expr> right): op(move(op)), right(move(right)) {}

Variable::Variable(Token name): name(move(name)) {}

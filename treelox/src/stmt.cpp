#include "stmt.h"

Expression::Expression(Expr expression): expression(expression) {}

If::If(Expr condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch):
	condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

Print::Print(Expr expression): expression(expression) {}

Var::Var(Token name, std::optional<Expr> initializer): name(name), initializer(initializer) {}

While::While(Expr condition, std::shared_ptr<Stmt> body): condition(condition), body(body) {}

Function::Function(Token name, std::vector<Token> params, std::vector<Stmt> body): name(name), params(params), body(body) {}

Block::Block(std::vector<Stmt> statements): statements(statements) {}

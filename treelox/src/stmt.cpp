#include "stmt.h"

Block::Block(std::vector<Stmt> statements): statements(statements) {}

Expression::Expression(Expr expression): expression(expression) {}

Function::Function(Token name, std::vector<Token> params, std::vector<Stmt> body): name(name), params(params), body(body) {}

If::If(Expr condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch):
	condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

Print::Print(Expr expression): expression(expression) {}

Return::Return(Token keyword, Expr value): keyword(keyword), value(value) {}

Var::Var(Token name, std::optional<Expr> initializer): name(name), initializer(initializer) {}

While::While(Expr condition, std::shared_ptr<Stmt> body): condition(condition), body(body) {}

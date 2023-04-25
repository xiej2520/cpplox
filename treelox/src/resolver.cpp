#include "resolver.h"

using std::string;
using std::unordered_map;
using std::vector;

using enum FunctionType;

Resolver::Resolver(Interpreter &it): it(it), current_function(NONE) {}

struct ResolverExprVisitor {
	Resolver &rs;
	ResolverExprVisitor(Resolver &rs): rs(rs) {}

	void operator()(std::monostate) {
		// do nothing
	}
	void operator()(const Assign &expr) {
		rs.resolve_expr(*expr.value);
		Assign &e = const_cast<Assign &>(expr);
		// actually resolve it
		rs.resolve_local(e, expr.name);
	}
	void operator()(const Binary &expr) {
		rs.resolve_expr(*expr.left);
		rs.resolve_expr(*expr.right);
	}
	void operator()(const Call &expr) {
		rs.resolve_expr(*expr.callee);
		for (const Expr &argument : expr.arguments) {
			rs.resolve_expr(argument);
		}
	}
	void operator()(const Get &expr) {
		// property dispatch is dynamic
		rs.resolve_expr(*expr.object);
	}
	void operator()(const Grouping &expr) {
		rs.resolve_expr(*expr.expression);
	}
	void operator()(const Literal &) {
		// do nothing
	}
	void operator()(const Logical &expr) {
		rs.resolve_expr(*expr.left);
		rs.resolve_expr(*expr.right);
	}
	void operator()(const Set &expr) {
		rs.resolve_expr(*expr.object);
		rs.resolve_expr(*expr.value);
	}
	void operator()(const Unary &expr) {
		rs.resolve_expr(*expr.right);
	}
	void operator()(const Variable &expr) {
		if (!rs.scopes.empty() && rs.scopes.back().contains(expr.name.lexeme)  && !rs.scopes.back()[expr.name.lexeme]) {
			Lox::error(expr.name, "Can't read local variable in its own initializer.");
		}
		// actually resolve it
		rs.resolve_local(expr, expr.name);
	}
};

struct ResolverStmtVisitor {
	Resolver &rs;
	ResolverStmtVisitor(Resolver &rs): rs(rs) {}

	void operator()(std::monostate) {
		// do nothing
	}
	void operator()(const Block &stmt) {
		rs.begin_scope();
		rs.resolve_block(stmt.statements);
		rs.end_scope();
	}
	void operator()(const Class &stmt) {
		rs.declare(stmt.name);
		rs.define(stmt.name);
		for (const Function &method : stmt.methods) {
			rs.resolve_function(method, METHOD);
		}
	}
	void operator()(const Expression &stmt) {
		rs.resolve_expr(stmt.expression);
	}
	void operator()(const Function &stmt) {
		rs.declare(stmt.name);
		rs.define(stmt.name);
		rs.resolve_function(stmt, FUNCTION);
	}
	void operator()(const If &stmt) {
		rs.resolve_expr(stmt.condition);
		rs.resolve_stmt(*stmt.then_branch);
		if (stmt.else_branch != nullptr) {
			rs.resolve_stmt(*stmt.else_branch);
		}
	}
	void operator()(const Print &stmt) {
		rs.resolve_expr(stmt.expression);
	}
	void operator()(const Return &stmt) {
		if (rs.current_function == NONE) {
			Lox::error(stmt.keyword, "Can't return from top-level code.");
		}
		if (!std::holds_alternative<std::monostate>(stmt.value)) {
			rs.resolve_expr(stmt.value);
		}
	}
	void operator()(const Var &stmt) {
		rs.declare(stmt.name);
		if (stmt.initializer != std::nullopt) {
			rs.resolve_expr(stmt.initializer.value());
		}
		rs.define(stmt.name);
	}
	void operator()(const While &stmt) {
		rs.resolve_expr(stmt.condition);
		rs.resolve_stmt(*stmt.body);
	}
};

void Resolver::begin_scope() {
	scopes.emplace_back();
}

void Resolver::end_scope() {
	scopes.pop_back();	
}

void Resolver::declare(Token name) {
	if (scopes.empty()) return;
	if (scopes.back().contains(name.lexeme)) {
		Lox::error(name, "Already a variable with this name in this scope");
	}
	scopes.back()[name.lexeme] = false;
}

void Resolver::define(Token name) {
	if (scopes.empty()) return;
	scopes.back()[name.lexeme] = true;
}

void Resolver::resolve_expr(const Expr &expr) {
	std::visit(ResolverExprVisitor(*this), expr);
}

#include <iostream>
void Resolver::resolve_stmt(const Stmt &stmt) {
	std::visit(ResolverStmtVisitor(*this), stmt);
}

void Resolver::resolve_block(const vector<Stmt> &statements) {
	for (const Stmt &statement : statements) {
		resolve_stmt(statement);
	}
}

void Resolver::resolve_local(const Assign &expr, const Token &name) {
	for (int i=scopes.size()-1; i>=0; i--) {
		if (scopes[i].contains(name.lexeme)) {
			//std::cout << "resolve local " << &expr << std::endl;
			it.resolve(expr, scopes.size() - 1 - i);
			return;
		}
	}
}

void Resolver::resolve_local(const Variable &expr, const Token &name) {
	for (int i=scopes.size()-1; i>=0; i--) {
		if (scopes[i].contains(name.lexeme)) {
			//std::cout << "resolve local " << &expr << std::endl;
			it.resolve(expr, scopes.size() - 1 - i);
			return;
		}
	}
}

void Resolver::resolve_function(const Function &fn, FunctionType type) {
	FunctionType enclosing_function = current_function;
	current_function = type;
	begin_scope();
	for (Token param : fn.params) {
		declare(param);
		define(param);
	}
	resolve_block(fn.body);
	end_scope();
	current_function = enclosing_function;
}

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
		rs.resolve(*expr.value);
		rs.resolve_local(expr, expr.name);
	}
	void operator()(const Binary &expr) {
		rs.resolve(*expr.left);
		rs.resolve(*expr.right);
	}
	void operator()(const Call &expr) {
		rs.resolve(*expr.callee);
		for (const Expr &argument : expr.arguments) {
			rs.resolve(argument);
		}
	}
	void operator()(const Grouping &expr) {
		rs.resolve(*expr.expression);
	}
	void operator()(const Literal &) {
		// do nothing
	}
	void operator()(const Logical &expr) {
		rs.resolve(*expr.left);
		rs.resolve(*expr.right);
	}
	void operator()(const Unary &expr) {
		rs.resolve(*expr.right);
	}
	void operator()(const Variable &expr) {
		if (!rs.scopes.empty() && !rs.scopes.back()[expr.name.lexeme]) {
			Lox::error(expr.name, "Can't read local variable in its own initializer.");
		}
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
		rs.resolve(stmt.statements);
		rs.end_scope();
	}
	void operator()(const Expression &stmt) {
		rs.resolve(stmt.expression);
	}
	void operator()(const Function &stmt) {
		rs.declare(stmt.name);
		rs.define(stmt.name);
		rs.resolve_function(stmt, FUNCTION);
	}
	void operator()(const If &stmt) {
		rs.resolve(stmt.condition);
		rs.resolve(*stmt.thenBranch);
		if (stmt.elseBranch != nullptr) {
			rs.resolve(*stmt.elseBranch);
		}
	}
	void operator()(const Print &stmt) {
		rs.resolve(stmt.expression);
	}
	void operator()(const Return &stmt) {
		if (rs.current_function == NONE) {
			Lox::error(stmt.keyword, "Can't return from top-level code.");
		}
		if (!std::holds_alternative<std::monostate>(stmt.value)) {
			rs.resolve(stmt.value);
		}
	}
	void operator()(const Var &stmt) {
		rs.declare(stmt.name);
		if (stmt.initializer != std::nullopt) {
			rs.resolve(stmt.initializer.value());
		}
		rs.define(stmt.name);
	}
	void operator()(const While &stmt) {
		rs.resolve(stmt.condition);
		rs.resolve(*stmt.body);
	}
};

void Resolver::begin_scope() {
	scopes.push_back(unordered_map<string, bool>());
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

void Resolver::resolve(const Expr &expr) {
	std::visit(ResolverExprVisitor(*this), expr);
}

void Resolver::resolve(const Stmt &stmt) {
	std::visit(ResolverStmtVisitor(*this), stmt);
}

void Resolver::resolve(const vector<Stmt> &statements) {
	for (Stmt statement : statements) {
		resolve(statement);
	}
}

void Resolver::resolve_local(const Expr &expr, Token name) {
	if (scopes.empty()) return;
	for (size_t i=scopes.size()-1; i>=0; i--) {
		if (scopes[i].contains(name.lexeme)) {
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
	resolve(fn.body);
	end_scope();
	current_function = enclosing_function;
}

#include "resolver.h"

using std::string;
using std::vector;

using enum FunctionType;

Resolver::Resolver(Interpreter &it): it(it) {}

void Resolver::operator()(std::monostate) {
	// do nothing
}
void Resolver::operator()(Assign &expr) {
	resolve(*expr.value);
	// actually resolve it
	resolve_local(expr, expr.name);
}
void Resolver::operator()(Binary &expr) {
	resolve(*expr.left);
	resolve(*expr.right);
}
void Resolver::operator()(Call &expr) {
	resolve(*expr.callee);
	for (Expr &argument : expr.arguments) {
		resolve(argument);
	}
}
void Resolver::operator()(Get &expr) {
	// property dispatch is dynamic
	resolve(*expr.object);
}
void Resolver::operator()(Grouping &expr) {
	resolve(*expr.expression);
}
void Resolver::operator()(Literal &) {
	// do nothing
}
void Resolver::operator()(Logical &expr) {
	resolve(*expr.left);
	resolve(*expr.right);
}
void Resolver::operator()(Set &expr) {
	resolve(*expr.object);
	resolve(*expr.value);
}
void Resolver::operator()(Super &expr) {
	if (current_class == ClassType::NONE) {
		Lox::error(expr.keyword, "Can't use 'super' outside of a class.");
	}
	else if (current_class != ClassType::SUBCLASS) {
		Lox::error(expr.keyword, "Can't use 'super' in a class with no superclass.");
	}
	resolve_local(expr, expr.keyword);
}
void Resolver::operator()(This &expr) {
	if (current_class == ClassType::NONE) {
		Lox::error(expr.keyword, "Can't use 'this' outside of a class.");
	}
	resolve_local(expr, expr.keyword);
}
void Resolver::operator()(Unary &expr) {
	resolve(*expr.right);
}
void Resolver::operator()(Variable &expr) {
	if (!scopes.empty() && scopes.back().contains(expr.name.lexeme)  && !scopes.back()[expr.name.lexeme]) {
		Lox::error(expr.name, "Can't read local variable in its own initializer.");
	}
	// actually resolve it
	resolve_local(expr, expr.name);
}

void Resolver::operator()(Block &stmt) {
	begin_scope();
	resolve(stmt.statements);
	end_scope();
}
void Resolver::operator()(Class &stmt) {
	ClassType enclosing_class(current_class);
	current_class = ClassType::CLASS;
	declare(stmt.name);
	define(stmt.name);
	if (stmt.superclass != nullptr && (stmt.name.lexeme == stmt.superclass->name.lexeme)) {
		Lox::error(stmt.superclass->name, "A class can't inherit from itself.");
	}
	if (stmt.superclass != nullptr) {
		current_class = ClassType::SUBCLASS;
		resolve(*stmt.superclass); // optional creates a value on the stack - fail
		begin_scope();
		scopes.back()["super"] = true;
	}
	begin_scope();
	scopes.back()["this"] = true;
	for (Function &method : stmt.methods) {
		resolve_function(method, method.name.lexeme == "init" ? INITIALIZER : METHOD);
	}
	end_scope();
	if (stmt.superclass != nullptr) {
		end_scope();
	}
	current_class = enclosing_class;
}
void Resolver::operator()(Expression &stmt) {
	resolve(stmt.expression);
}
void Resolver::operator()(Function &stmt) {
	declare(stmt.name);
	define(stmt.name);
	resolve_function(stmt, FUNCTION);
}
void Resolver::operator()(If &stmt) {
	resolve(stmt.condition);
	resolve(*stmt.then_branch);
	if (stmt.else_branch != nullptr) {
		resolve(*stmt.else_branch);
	}
}
void Resolver::operator()(Print &stmt) {
	resolve(stmt.expression);
}
void Resolver::operator()(Return &stmt) {
	if (current_function == NONE) {
		Lox::error(stmt.keyword, "Can't return from top-level code.");
	}
	if (!std::holds_alternative<std::monostate>(stmt.value)) {
		if (current_function == INITIALIZER) {
			Lox::error(stmt.keyword, "Can't return a value from an initializer.");
		}
		resolve(stmt.value);
	}
}
void Resolver::operator()(Var &stmt) {
	declare(stmt.name);
	if (stmt.initializer != std::nullopt) {
		resolve(stmt.initializer.value());
	}
	define(stmt.name);
}
void Resolver::operator()(While &stmt) {
	resolve(stmt.condition);
	resolve(*stmt.body);
}

void Resolver::begin_scope() {
	scopes.emplace_back();
}

void Resolver::end_scope() {
	scopes.pop_back();	
}

void Resolver::declare(Token name) {
	if (scopes.empty()) return;
	if (scopes.back().contains(name.lexeme)) {
		Lox::error(name, "Already a variable with this name in this scope.");
	}
	scopes.back()[name.lexeme] = false;
}

void Resolver::define(Token name) {
	if (scopes.empty()) return;
	scopes.back()[name.lexeme] = true;
}

void Resolver::resolve(Expr &expr) {
	std::visit(*this, expr);
}

void Resolver::resolve(Variable &expr) {
	(*this)(expr);
}

void Resolver::resolve(Stmt &stmt) {
	std::visit(*this, stmt);
}

void Resolver::resolve(vector<Stmt> &statements) {
	for (Stmt &statement : statements) {
		resolve(statement);
	}
}

// Assign, Super, This, Variable
template<class T>
void Resolver::resolve_local(T &expr, const Token &name) {
	for (int i=scopes.size()-1; i>=0; i--) {
		if (scopes[i].contains(name.lexeme)) {
			it.resolve(expr, scopes.size() - 1 - i);
			return;
		}
	}
}

void Resolver::resolve_function(Function &fn, FunctionType type) {
	FunctionType enclosing_function = current_function;
	current_function = type;
	begin_scope();
	for (const Token &param : fn.params) {
		declare(param);
		define(param);
	}
	resolve(fn.body);
	end_scope();
	current_function = enclosing_function;
}

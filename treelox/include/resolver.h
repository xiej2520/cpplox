#pragma once

#include <unordered_map>
#include <vector>
#include "interpreter.h"
#include "stmt.h"

enum class FunctionType {
	NONE,
	FUNCTION
};

struct Resolver {
	Interpreter &it;
	std::vector<std::unordered_map<std::string, bool>> scopes;
	FunctionType current_function;
	Resolver(Interpreter &it);
	void begin_scope();
	void end_scope();
	void declare(Token name);
	void define(Token name);
	void resolve_expr(const Expr &expr);
	void resolve_stmt(const Stmt &stmt);
	void resolve_block(const std::vector<Stmt> &statements);

	// overloading to const Expr & creates a copy on the stack??? doesn't work
	void resolve_local(const Assign &expr, const Token &name);
	void resolve_local(const Variable &expr, const Token &name);

	void resolve_function(const Function &fn, FunctionType type);
};

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
	void resolve(const Expr &expr);
	void resolve(const Stmt &stmt);
	void resolve(const std::vector<Stmt> &statements);
	void resolve_local(const Expr &expr, Token name);
	void resolve_function(const Function &fn, FunctionType type);
};

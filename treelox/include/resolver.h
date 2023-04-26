#pragma once

#include <unordered_map>
#include <vector>
#include "interpreter.h"
#include "stmt.h"

enum class FunctionType {
	NONE,
	FUNCTION,
	INITIALIZER,
	METHOD
};

enum class ClassType {
	NONE,
	CLASS,
	SUBCLASS
};

struct Resolver {
	Interpreter &it;
	std::vector<std::unordered_map<std::string, bool>> scopes;
	FunctionType current_function = FunctionType::NONE;
	ClassType current_class = ClassType::NONE;
	Resolver(Interpreter &it);
	void begin_scope();
	void end_scope();
	void declare(Token name);
	void define(Token name);
	void resolve(Expr &expr);
	void resolve(Variable &expr);
	void resolve(Stmt &stmt);
	void resolve(std::vector<Stmt> &statements);
	
	void operator()(std::monostate);
	void operator()(Assign &expr);
	void operator()(Binary &expr);
	void operator()(Call &expr);
	void operator()(Get &expr);
	void operator()(Grouping &expr);
	void operator()(Literal &expr);
	void operator()(Logical &expr);
	void operator()(Set &expr);
	void operator()(Super &expr);
	void operator()(This &expr);
	void operator()(Unary &expr);
	void operator()(Variable &expr);
	void operator()(Block &stmt);
	void operator()(Class &stmt);
	void operator()(Expression &stmt);
	void operator()(Function &stmt);
	void operator()(If &stmt);
	void operator()(Print &stmt);
	void operator()(Return &stmt);
	void operator()(Var &stmt);
	void operator()(While &stmt);

	// overloading to const Expr & creates a copy on the stack??? doesn't work
	template<class T>
	void resolve_local(T &expr, const Token &name);

	void resolve_function(Function &fn, FunctionType type);
};

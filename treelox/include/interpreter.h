#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <variant>
#include <vector>

#include "token.h"
#include "expr.h"
#include "stmt.h"
#include "lox.h"
#include "lox_object.h"
#include "environment.h"
#include "runtime_error.h"

class Interpreter {
	/* Constructor requires an Interpreter argument
	 * Use with std::visit
	 * takes a Expr expr argument, evaluates and returns final LoxObject
	 */
	struct EvaluateExpr;
	/* Constructor requires an Interpreter argument
	 * Use with std::visit
	 * takes a Stmt stmt argument, runs it with interpreter
	 */
	struct EvaluateStmt;
	std::unordered_map<Variable *, int> locals;
	// takes in an Expr, evaluates it down to LoxObject
	LoxObject evaluate(const Expr &expr);
	
	// only true value is bool true, everything else is false
	bool is_truthy(const LoxObject &obj);

	void check_num_operand(Token op, LoxObject operand);
	void check_num_operands(Token op, LoxObject left, LoxObject right);

public:
	std::shared_ptr<Environment> globals;
	std::shared_ptr<Environment> environment;
	LoxObject return_value = std::monostate{};
	bool has_return = false;

	Interpreter();

	void interpret(const std::vector<Stmt> &expression);
	void repl_interpret(const std::vector<Stmt> &expression);
	
	// not private for functions
	void execute(const Stmt &stmt);
	void execute_block(const std::vector<Stmt> &statements, std::shared_ptr<Environment> environment);
	
	template<class T>
	void resolve(T &expr, int depth);
	// using const Expr &expr instead of const Variable &expr creates a temporary on the stack?
	template<class T>
	LoxObject look_up_variable(const Token &name, T &expr);
};

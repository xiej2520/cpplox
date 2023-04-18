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

	// takes in an Expr, evaluates it down to LoxObject
	LoxObject evaluate(Expr expr);
	
	bool is_equal(LoxObject a, LoxObject b);
	// only true value is bool true, everything else is false
	bool is_truthy(LoxObject obj);

	void check_num_operand(Token op, LoxObject operand);
	void check_num_operands(Token op, LoxObject left, LoxObject right);

public:
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
	const std::shared_ptr<Environment> globals;
	std::shared_ptr<Environment> environment;
	Interpreter();
	void interpret(const std::vector<Stmt> &expression);
	void repl_interpret(const std::vector<Stmt> &expression);
	
	// not private for functions
	void execute(const Stmt &stmt);
	void execute_block(const std::vector<Stmt> &statements, std::shared_ptr<Environment> environment);
};

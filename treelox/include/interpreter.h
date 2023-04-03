#pragma once

#include <iostream>
#include <variant>
#include "token.h"
#include "expr.h"
#include "stmt.h"
#include "treelox.h"
#include "environment.h"
#include "runtime_error.h"

class Interpreter {
	struct TreeLoxCallable {
		LiteralVar call(Interpreter &it, std::vector<Expr> arguments);
	};
	std::shared_ptr<Environment> environment;

	// takes in an Expr, evaluates it down to final LiteralVar
	LiteralVar evaluate(Expr expr);
	
	TreeLoxCallable getCallable(LiteralVar callee);

	bool is_equal(LiteralVar a, LiteralVar b);
	// only true value is bool true, everything else is false
	bool is_truthy(LiteralVar literal);

	void check_num_operand(Token op, LiteralVar operand);
	void check_num_operands(Token op, LiteralVar left, LiteralVar right);
	
	void execute(const Stmt &stmt);
	void execute_block(const std::vector<Stmt> &statements, Environment environment);

public:
	/* Constructor requires an Interpreter argument
	 * Use with std::visit
	 * takes a Expr expr argument, evaluates and returns final LiteralVar
	 */
	struct EvaluateExpr;
	/* Constructor requires an Interpreter argument
	 * Use with std::visit
	 * takes a Stmt stmt argument, runs it with interpreter
	 */
	struct EvaluateStmt;
	Interpreter();
	void interpret(const std::vector<Stmt> &expression);
	void repl_interpret(const std::vector<Stmt> &expression);
};

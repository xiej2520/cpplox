#include "interpreter.h"
#include "stmt.h"

using enum TokenType;

struct Interpreter::EvaluateExpr {
	Interpreter &it;
	EvaluateExpr(Interpreter &it): it(it) {}

	LiteralVar operator()(std::monostate m) {
		return std::monostate{};
	}
	LiteralVar operator()(Literal expr) {
		return expr.value;
	}
	LiteralVar operator()(Grouping expr) {
		return it.evaluate(*expr.expression);
	}
	LiteralVar operator()(Unary expr) {
		LiteralVar right = it.evaluate(*expr.right);
		switch (expr.op.type) {
			case MINUS:
				it.check_num_operand(expr.op, right);
				return -std::get<double>(right);
			case BANG:
				return !it.is_truthy(right);
			default:
				return std::monostate{};
		}
		return std::monostate{}; // unreachable
	}
	LiteralVar operator()(Assign expr) {
		LiteralVar value = it.evaluate(expr.value);
		it.environment->assign(expr.name, value);
		return value;
	}
	LiteralVar operator()(Binary expr) {
		LiteralVar left = it.evaluate(*expr.left);
		LiteralVar right = it.evaluate(*expr.right);
		switch (expr.op.type) {
			case PLUS:
				if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
					return std::get<double>(left) + std::get<double>(right);
				}
				if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
					return std::get<std::string>(left) + std::get<std::string>(right);
				}
				throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
			case MINUS:
				it.check_num_operands(expr.op, left, right);
				return std::get<double>(left) - std::get<double>(right);
			case SLASH:
				it.check_num_operands(expr.op, left, right);
				if (std::get<double>(right) == 0.0) {
					throw RuntimeError(expr.op, "Division by zero error.");
				}
				return std::get<double>(left) / std::get<double>(right);
			case STAR:
				it.check_num_operands(expr.op, left, right);
				return std::get<double>(left) * std::get<double>(right);
				// NO TYPE COERCION
			case GREATER:
				it.check_num_operands(expr.op, left, right);
				return std::get<double>(left) > std::get<double>(right);
			case GREATER_EQUAL:
				it.check_num_operands(expr.op, left, right);
				return std::get<double>(left) >= std::get<double>(right);
			case LESS:
				it.check_num_operands(expr.op, left, right);
				return std::get<double>(left) < std::get<double>(right);
			case LESS_EQUAL:
				it.check_num_operands(expr.op, left, right);
				return std::get<double>(left) <= std::get<double>(right);
			case BANG_EQUAL:
				return !it.is_equal(left, right);
			case EQUAL_EQUAL:
				return !it.is_equal(left, right);
			default:
				return std::monostate{};
		}
		return std::monostate{};
	}
	LiteralVar operator()(Call expr) {
		LiteralVar callee = it.evaluate(*expr.callee);
		std::vector<LiteralVar> arguments;
		for (Expr argument : expr.arguments) {
			arguments.push_back(it.evaluate(argument));
		}
		TreeLoxCallable function = getCallable(callee);
		return function.call(it, arguments);
	}
	LiteralVar operator()(Logical expr) {
		LiteralVar left = it.evaluate(expr.left);
		if (expr.op.type == OR) {
			// short circuit
			if (it.is_truthy(left)) return left;
			else {
				if (!it.is_truthy(left)) return left;
			}
		}
		return it.evaluate(expr.right);
	}
	LiteralVar operator()(Variable expr) {
		return it.environment->get(expr.name);
	}
};

struct Interpreter::EvaluateStmt {
	Interpreter &it;
	EvaluateStmt(Interpreter &it): it(it) {}

	void operator()(std::monostate m) {}
	void operator()(Block stmt) {
		it.execute_block(stmt.statements, Environment(*it.environment));
	}
	void operator()(Expression stmt) {
		it.evaluate(stmt.expression);
	}
	void operator()(If stmt) {
		if (it.is_truthy(it.evaluate(stmt.condition))) {
			it.execute(*stmt.thenBranch);
		}
		else if (!std::holds_alternative<std::monostate>(*stmt.elseBranch)) {
			it.execute(*stmt.elseBranch);
		}
	}
	void operator()(Print stmt) {
		LiteralVar value = it.evaluate(stmt.expression);
		std::cout << std::visit(LiteralToString(), value) << "\n";
	}
	void operator()(Var stmt) {
		if (stmt.initializer.has_value()) {
			it.environment->define(stmt.name.lexeme, it.evaluate(stmt.initializer.value()));
		}
		else {
			it.environment->define_uninitialized(stmt.name.lexeme);
		}
	}
	void operator()(While stmt) {
		while (it.is_truthy((it.evaluate(stmt.condition)))) {
			it.execute(*stmt.body);
		}
	}
};

Interpreter::Interpreter(): environment(std::make_shared<Environment>()) { }

LiteralVar Interpreter::evaluate(Expr expr) {
	return std::visit(EvaluateExpr(*this), expr);
}

bool Interpreter::is_equal(LiteralVar a, LiteralVar b) {
	return a == b;
}

bool Interpreter::is_truthy(LiteralVar literal) {
	if (std::holds_alternative<bool>(literal)) {
		return std::get<bool>(literal);
	}
	return false;
}

void Interpreter::check_num_operand(Token op, LiteralVar operand) {
	if (std::holds_alternative<double>(operand)) return;
	throw RuntimeError(op, "Operand must be a number.");
}

void Interpreter::check_num_operands(Token op, LiteralVar left, LiteralVar right) {
	if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) return;
	throw RuntimeError(op, "Operands must be a numbers.");
}

void Interpreter::execute(const Stmt &stmt) {
	std::visit(EvaluateStmt(*this), stmt);
}

void Interpreter::execute_block(const std::vector<Stmt> &statements, Environment new_env) {
	auto prev_env = this->environment;
	try {
		this->environment = std::make_shared<Environment>(new_env);
		for (const Stmt &statement : statements) {
			execute(statement);
		}
	}
	catch (RuntimeError &err) { }
	// no finally
	this->environment = prev_env;
}

void Interpreter::interpret(const std::vector<Stmt> &statements) {
	try {
		for (const Stmt &statement : statements) {
			execute(statement);
		}
	}
	catch (RuntimeError &err) {
		TreeLox::runtime_error(err);
	}
}

// this won't print expressions in {blocks} because they're statements
void Interpreter::repl_interpret(const std::vector<Stmt> &statements) {
	try {
		for (const Stmt &statement : statements) {
			if (std::holds_alternative<Expression>(statement)) {
				LiteralVar value = evaluate(std::get<Expression>(statement).expression);
				std::cout << std::visit(LiteralToString(), value) << "\n";
			}
			else {
				execute(statement);
			}
		}
	}
	catch (RuntimeError &err) {
		TreeLox::runtime_error(err);
	}
}


RuntimeError::RuntimeError(Token token, const char *msg): token(token), msg(msg) { }
RuntimeError::RuntimeError(Token token, std::string &msg): token(token), msg(msg) { }

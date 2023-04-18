#include "interpreter.h"

using std::make_shared;
using std::shared_ptr;
using std::holds_alternative;
using std::get;
using std::vector;

using enum TokenType;

struct Interpreter::EvaluateExpr {
	Interpreter &it;
	EvaluateExpr(Interpreter &it): it(it) {}

	LoxObject operator()(std::monostate) {
		return std::monostate{};
	}
	LoxObject operator()(Literal expr) {
		return expr.value;
	}
	LoxObject operator()(Grouping expr) {
		return it.evaluate(*expr.expression);
	}
	LoxObject operator()(Unary expr) {
		LoxObject right = it.evaluate(*expr.right);
		switch (expr.op.type) {
			case MINUS:
				it.check_num_operand(expr.op, right);
				return -get<double>(right);
			case BANG:
				return !it.is_truthy(right);
			default:
				return std::monostate{};
		}
		return std::monostate{}; // unreachable
	}
	LoxObject operator()(Assign expr) {
		LoxObject value = it.evaluate(expr.value);
		it.environment->assign(expr.name, value);
		return value;
	}
	LoxObject operator()(Binary expr) {
		LoxObject left = it.evaluate(*expr.left);
		LoxObject right = it.evaluate(*expr.right);
		switch (expr.op.type) {
			case PLUS:
				if (holds_alternative<double>(left) && holds_alternative<double>(right)) {
					return get<double>(left) + std::get<double>(right);
				}
				if (holds_alternative<std::string>(left) && holds_alternative<std::string>(right)) {
					return get<std::string>(left) + std::get<std::string>(right);
				}
				throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
			case MINUS:
				it.check_num_operands(expr.op, left, right);
				return get<double>(left) - std::get<double>(right);
			case SLASH:
				it.check_num_operands(expr.op, left, right);
				if (get<double>(right) == 0.0) {
					throw RuntimeError(expr.op, "Division by zero error.");
				}
				return get<double>(left) / std::get<double>(right);
			case STAR:
				it.check_num_operands(expr.op, left, right);
				return get<double>(left) * std::get<double>(right);
				// NO TYPE COERCION
			case GREATER:
				it.check_num_operands(expr.op, left, right);
				return get<double>(left) > std::get<double>(right);
			case GREATER_EQUAL:
				it.check_num_operands(expr.op, left, right);
				return get<double>(left) >= std::get<double>(right);
			case LESS:
				it.check_num_operands(expr.op, left, right);
				return get<double>(left) < std::get<double>(right);
			case LESS_EQUAL:
				it.check_num_operands(expr.op, left, right);
				return get<double>(left) <= std::get<double>(right);
			case BANG_EQUAL:
				return !it.is_equal(left, right);
			case EQUAL_EQUAL:
				return !it.is_equal(left, right);
			default:
				return std::monostate{};
		}
		return std::monostate{};
	}
	LoxObject operator()(Call expr) {
		LoxObject callee = it.evaluate(*expr.callee);

		vector<LoxObject> arguments;
		for (Expr argument : expr.arguments) {
			arguments.push_back(it.evaluate(argument));
		}

		if (!holds_alternative<LoxFunction>(callee)) {
			throw RuntimeError(expr.paren, "Can only call functions and classes.");
		}
		if (arguments.size() != get<LoxFunction>(callee).arity) {
			std::string msg = "Expected " + to_string(static_cast<int>(get<LoxFunction>(callee).arity))
			+ " arguments but got " + to_string(static_cast<int>(arguments.size())) + ".";
			throw RuntimeError(expr.paren, msg);
		}

		return get<LoxFunction>(callee).operator()(it, arguments);
	}
	LoxObject operator()(Logical expr) {
		LoxObject left = it.evaluate(expr.left);
		if (expr.op.type == OR) {
			// short circuit
			if (it.is_truthy(left)) return left;
			else {
				if (!it.is_truthy(left)) return left;
			}
		}
		return it.evaluate(expr.right);
	}
	LoxObject operator()(Variable expr) {
		return it.environment->get(expr.name);
	}
};

struct Interpreter::EvaluateStmt {
	Interpreter &it;
	EvaluateStmt(Interpreter &it): it(it) {}

	void operator()(std::monostate) {}
	void operator()(Block stmt) {
		it.execute_block(stmt.statements, make_shared<Environment>(it.environment));
	}
	void operator()(Expression stmt) {
		it.evaluate(stmt.expression);
	}
	void operator()(Function stmt) {
		LoxFunction fn = LoxFunction(make_shared<Function>(stmt));
		it.environment->define(stmt.name.lexeme, fn);
	}
	void operator()(If stmt) {
		if (it.is_truthy(it.evaluate(stmt.condition))) {
			it.execute(*stmt.thenBranch);
		}
		else if (!holds_alternative<std::monostate>(*stmt.elseBranch)) {
			it.execute(*stmt.elseBranch);
		}
	}
	void operator()(Print stmt) {
		LoxObject value = it.evaluate(stmt.expression);
		std::cout << to_string(value) << "\n";
	}
	void operator()(Return stmt) {
		LoxObject value = holds_alternative<std::monostate>(stmt.value) ? std::monostate{} : it.evaluate(stmt.value);
		// lol
		// buggy
		// exceptions bad
		throw ReturnUnwind(value);
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

Interpreter::Interpreter(): globals(make_shared<Environment>()) {
	environment = globals;
	globals->define("clock", NativeFunction(0, [](Interpreter &, const std::vector<LoxObject> &) {
		return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}));
}

LoxObject Interpreter::evaluate(Expr expr) {
	return std::visit(EvaluateExpr(*this), expr);
}

bool Interpreter::is_equal(LoxObject a, LoxObject b) {
	return a == b;
}

bool Interpreter::is_truthy(LoxObject obj) {
	if (holds_alternative<bool>(obj)) {
		return get<bool>(obj);
	}
	return false;
}

void Interpreter::check_num_operand(Token op, LoxObject operand) {
	if (holds_alternative<double>(operand)) return;
	throw RuntimeError(op, "Operand must be a number.");
}

void Interpreter::check_num_operands(Token op, LoxObject left, LoxObject right) {
	if (holds_alternative<double>(left) && holds_alternative<double>(right)) return;
	throw RuntimeError(op, "Operands must be a numbers.");
}

void Interpreter::execute(const Stmt &stmt) {
	std::visit(EvaluateStmt(*this), stmt);
}

void Interpreter::execute_block(const vector<Stmt> &statements, shared_ptr<Environment> new_env) {
	auto prev_env = this->environment;
	try {
		this->environment = new_env;
		for (const Stmt &statement : statements) {
			execute(statement);
		}
	}
	catch (RuntimeError &err) { }
	// no finally, MUST CATCH OTHER EXCEPTIONS ELSEWHERE AND RESET this->environment (e.g. LoxFunction)
	this->environment = prev_env;
}

void Interpreter::interpret(const std::vector<Stmt> &statements) {
	try {
		for (const Stmt &statement : statements) {
			execute(statement);
		}
	}
	catch (RuntimeError &err) {
		Lox::runtime_error(err);
	}
}

// this won't print expressions in {blocks} because they're statements
void Interpreter::repl_interpret(const std::vector<Stmt> &statements) {
	try {
		for (const Stmt &statement : statements) {
			if (holds_alternative<Expression>(statement)) {
				LoxObject value = evaluate(get<Expression>(statement).expression);
				std::cout << to_string(value) << "\n";
			}
			else {
				execute(statement);
			}
		}
	}
	catch (RuntimeError &err) {
		Lox::runtime_error(err);
	}
}


RuntimeError::RuntimeError(Token token, const char *msg): token(token), msg(msg) { }
RuntimeError::RuntimeError(Token token, std::string &msg): token(token), msg(msg) { }

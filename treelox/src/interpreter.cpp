#include "interpreter.h"

using std::make_unique;
using std::make_shared;
using std::holds_alternative;
using std::get;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

using enum TokenType;

struct Interpreter::EvaluateExpr {
	Interpreter &it;
	EvaluateExpr(Interpreter &it): it(it) {}

	LoxObject operator()(std::monostate m) {
		return m;
	}
	LoxObject operator()(const Assign &expr) {
		LoxObject value = it.evaluate(*expr.value);
		if (it.locals.contains((Variable *)(&expr))) {
			it.environment->assign_at(it.locals[(Variable *)(&expr)], expr.name, value);
		}
		else {
			it.globals.assign(expr.name, value);
		}
		return value;
	}
	LoxObject operator()(const Binary &expr) {
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
				// NO TYPE COERCION
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
				return !(left == right);
			case EQUAL_EQUAL:
				return left == right;
			default:
				return std::monostate{};
		}
		return std::monostate{};
	}
	LoxObject operator()(const Call &expr) {
		LoxObject callee = it.evaluate(*expr.callee);

		vector<LoxObject> arguments;
		for (const Expr &argument : expr.arguments) {
			arguments.push_back(it.evaluate(argument));
		}
		
		if (holds_alternative<LoxFunction>(callee)) {
			if (arguments.size() != get<LoxFunction>(callee).arity) {
				std::string msg = "Expected " + to_string(static_cast<int>(get<LoxFunction>(callee).arity))
				+ " arguments but got " + to_string(static_cast<int>(arguments.size())) + ".";
				throw RuntimeError(expr.paren, msg);
			}
			return get<LoxFunction>(callee).operator()(it, arguments);
		}
		if (holds_alternative<NativeFunction>(callee)) {
			if (arguments.size() != get<NativeFunction>(callee).arity) {
				std::string msg = "Expected " + to_string(static_cast<int>(get<NativeFunction>(callee).arity))
				+ " arguments but got " + to_string(static_cast<int>(arguments.size())) + ".";
				throw RuntimeError(expr.paren, msg);
			}
			return get<NativeFunction>(callee).operator()(it, arguments);
		}
		if (holds_alternative<shared_ptr<LoxClass>>(callee)) {
			// No long valid due to using shared_ptr
			// watch out for callee being deallocated from stack if rewriting!
			return make_shared<LoxInstance>((*get<shared_ptr<LoxClass>>(callee))(it, arguments));
		}
		throw RuntimeError(expr.paren, "Can only call functions and classes.");
	}
	LoxObject operator()(const Get &expr) {
		LoxObject obj = it.evaluate(*expr.object);
		if (holds_alternative<shared_ptr<LoxInstance>>(obj)) {
			return get<shared_ptr<LoxInstance>>(obj)->get(expr.name);
		}
		throw RuntimeError(expr.name, "Only instances have properties.");
	}
	LoxObject operator()(const Grouping &expr) {
		return it.evaluate(*expr.expression);
	}
	LoxObject operator()(const Literal &expr) {
		return LoxObject(expr.value);
	}
	LoxObject operator()(const Logical &expr) {
		LoxObject left = it.evaluate(*expr.left);
		if (expr.op.type == OR) {
			// short circuit
			if (it.is_truthy(left)) return left;
		}
		else { // AND
			if (!it.is_truthy(left)) return left;
		}
		return it.evaluate(*expr.right);
	}
	LoxObject operator()(const Set &expr) {
		LoxObject obj(it.evaluate(*expr.object));
		if (!holds_alternative<shared_ptr<LoxInstance>>(obj)) {
			throw RuntimeError(expr.name, "Only instances have fields.");
		}
		
		LoxObject value(it.evaluate(*expr.value));
		get<shared_ptr<LoxInstance>>(obj)->set(expr.name, value);
		return value;
	}
	LoxObject operator()(const Super &expr) {
		int distance = it.locals[(Variable *) const_cast<Super *>(&expr)];

		auto superclass = get<shared_ptr<LoxClass>>(it.environment->get_at(distance, "super"));
		auto obj = get<shared_ptr<LoxInstance>>(it.environment->get_at(distance-1, "this"));

		auto opt_method = superclass->find_method(expr.method.lexeme);
		if (!opt_method.has_value()) {
			throw RuntimeError(expr.method, "Undefined property '" + expr.method.lexeme + "'.");
		}
		return opt_method.value().bind(obj.get());
	}
	LoxObject operator()(const This &expr) {
		return it.look_up_variable(expr.keyword, expr);
	}
	LoxObject operator()(const Unary &expr) {
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
	LoxObject operator()(const Variable &expr) {
	//std::cout << "Alooking up variable " << expr.name.lexeme << " |" << &expr << std::endl;
		//std::cout << "got variable " << to_string(it.look_up_variable(expr.name, expr)) << " from lookup" << std::endl;
		return it.look_up_variable(expr.name, expr);
	}
};

struct Interpreter::EvaluateStmt {
	Interpreter &it;
	EvaluateStmt(Interpreter &it): it(it) {}

	void operator()(std::monostate) {}
	void operator()(const Block &stmt) {
		it.execute_block(stmt.statements, make_unique<Environment>(it.environment).get());
	}
	void operator()(const Class &stmt) {
		LoxObject superclass = shared_ptr<LoxClass>(nullptr);
		if (stmt.superclass.has_value()) {
			superclass = it.evaluate(stmt.superclass.value());
			if (!holds_alternative<shared_ptr<LoxClass>>(superclass)) {
				throw RuntimeError(stmt.superclass.value().name, "Superclass must be a class.");
			}
		}
		it.environment->define_uninitialized(stmt.name.lexeme);
		shared_ptr<Environment> super_closure = nullptr;
		if (stmt.superclass.has_value()) {
			super_closure = make_shared<Environment>(it.environment);
			it.environment = super_closure.get();
			it.environment->define("super", superclass);
		}
		unordered_map<string, LoxFunction> methods;
		for (const Function &method : stmt.methods) {
			LoxFunction method_instance(&method, it.environment);
			method_instance.is_initializer = method.name.lexeme == "init";
			methods.emplace(method.name.lexeme, method_instance);
		}
		auto klass = make_shared<LoxClass>(stmt.name.lexeme, get<shared_ptr<LoxClass>>(superclass), methods);
		if (get<shared_ptr<LoxClass>>(superclass) != nullptr) {
			it.environment = it.environment->enclosing;
			klass->super_closure = super_closure;
		}
		it.environment->assign(stmt.name, klass);
	}
	void operator()(const Expression &stmt) {
		it.evaluate(stmt.expression);
	}
	void operator()(const Function &stmt) {
		LoxFunction fn = LoxFunction(&stmt, it.environment);
		it.environment->define(stmt.name.lexeme, fn);
	}
	void operator()(const If &stmt) {
		if (it.is_truthy(it.evaluate(stmt.condition))) {
			it.execute(*stmt.then_branch);
		}
		else if (!holds_alternative<std::monostate>(*stmt.else_branch)) {
			it.execute(*stmt.else_branch);
		}
	}
	void operator()(const Print &stmt) {
		LoxObject value = it.evaluate(stmt.expression);
		std::cout << to_string(value) << "\n";
	}
	void operator()(const Return &stmt) {
		LoxObject value = holds_alternative<std::monostate>(stmt.value) ? std::monostate{} : it.evaluate(stmt.value);
		// lol
		// buggy
		// exceptions bad
		// this is INSANELY slow, at least for the fibonacci benchmark
		throw ReturnUnwind(value);
	}
	void operator()(Var &stmt) {
		if (stmt.initializer.has_value()) {
			it.environment->define(stmt.name.lexeme, it.evaluate(stmt.initializer.value()));
		}
		else {
			it.environment->define_uninitialized(stmt.name.lexeme);
		}
	}
	void operator()(const While &stmt) {
		while (it.is_truthy(it.evaluate(stmt.condition))) {
			it.execute(*stmt.body);
		}
	}
};

Interpreter::Interpreter(): environment(&globals) {
	globals.define("clock", NativeFunction(0, [](Interpreter &, const std::vector<LoxObject> &) {
		return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}));
	globals.define("str", NativeFunction(1, [](Interpreter &, const std::vector<LoxObject> args) {
		if (args.size() != 1 || (!holds_alternative<double>(args[0]) && !holds_alternative<int>(args[0]))) {
			throw RuntimeError(Token(NIL, "str", std::monostate{}, 0), "Expected one number as argument.");
		}
		if (holds_alternative<double>(args[0])) {
			return to_string(get<double>(args[0]));
		}
		return to_string(get<int>(args[0]));
	}));
}

LoxObject Interpreter::evaluate(const Expr &expr) {
	return std::visit(EvaluateExpr(*this), expr);
}

bool Interpreter::is_truthy(const LoxObject &obj) {
	if (holds_alternative<bool>(obj)) {
		return get<bool>(obj);
	}
	return false;
}

void Interpreter::check_num_operand(Token op, LoxObject operand) {
	if (holds_alternative<double>(operand)) {
		return;
	}
	throw RuntimeError(op, "Operand must be a number.");
}

void Interpreter::check_num_operands(Token op, LoxObject left, LoxObject right) {
	if (holds_alternative<double>(left) && holds_alternative<double>(right)) {
		return;
	}
	throw RuntimeError(op, "Operands must be a numbers.");
}

void Interpreter::execute(const Stmt &stmt) {
			//std::cout << "execute " << &stmt << std::endl;
	std::visit(EvaluateStmt(*this), const_cast<Stmt &>(stmt));
}

void Interpreter::execute_block(const vector<Stmt> &statements, Environment *new_env) {
	struct SaveEnvironment {
		Interpreter &it;
		Environment *prev_env;
		SaveEnvironment(Interpreter &it): it(it), prev_env(it.environment)  { }
		~SaveEnvironment() { it.environment = prev_env; }
	};
	SaveEnvironment push_env(*this);
	try {
		environment = new_env;
		for (const Stmt &statement : statements) {
			execute(statement);
		}
	}
	catch (RuntimeError &err) { }
	// RAII instead of finally block. Otherwise, must catch ReturnUnwind in LoxFunction
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
			execute(statement);
		}
	}
	catch (RuntimeError &err) {
		Lox::runtime_error(err);
	}
}

void Interpreter::resolve(const Assign &expr, int depth) {
	Variable *key = (Variable *) const_cast<Assign *>(&expr);
	locals[key] = depth;
	//std::cout << "binding expr " << to_string(expr.name) << " to " << key << " depth " << depth << " addr" << &expr << std::endl;
}

void Interpreter::resolve(const Super &expr, int depth) {
	Variable *key = (Variable *) const_cast<Super *>(&expr);
	locals[key] = depth;
}

void Interpreter::resolve(const This &expr, int depth) {
	Variable *key = (Variable *) const_cast<This *>(&expr);
	locals[key] = depth;
}

void Interpreter::resolve(const Variable &expr, int depth) {
	Variable *key = (Variable *) const_cast<Variable *>(&expr);
	locals[key] = depth;
	//std::cout << "binding expr " << to_string(expr) << " to " << key << " depth " << depth << " addr " << &expr << std::endl;
}

LoxObject Interpreter::look_up_variable(Token name, const This &expr) {
	if (locals.contains((Variable *) const_cast<This *>(&expr))) {
		return environment->get_at(locals[(Variable *)const_cast<This *>(&expr)], name.lexeme);
	}
	else {
		return globals.get(name);
	}
}

LoxObject Interpreter::look_up_variable(Token name, const Variable &expr) {
	//std::cout << "looking up variable " << to_string(expr) << " |addr| " << &expr << std::endl;
	if (locals.contains(const_cast<Variable *>(&expr))) {
		return environment->get_at(locals[const_cast<Variable *>(&expr)], name.lexeme);
	}
	else {
		//std::cout << "local var not found, looking in globals" << std::endl;
		return globals.get(name);
	}
}

RuntimeError::RuntimeError(Token token, const char *msg): token(token), msg(msg) { }
RuntimeError::RuntimeError(Token token, const std::string &msg): token(token), msg(msg) { }

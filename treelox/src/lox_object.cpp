#include "environment.h"
#include "interpreter.h"
#include "lox_object.h"
#include "stmt.h"

using std::function;
using std::make_unique;
using std::make_shared;
using std::optional;
using std::shared_ptr;
using std::string;
using std::string_view;
using std::unordered_map;
using std::vector;


LoxFunction::LoxFunction(const Function *declaration, Environment *closure):
	declaration(declaration), closure(closure), bound_closure(nullptr), arity(declaration->params.size()) { }

LoxFunction LoxFunction::bind(LoxInstance *instance) {
	// create new method instance
	LoxFunction method_instance(declaration, nullptr);
	// bind the closure to the method (use shared_ptr to keep it alive)
	method_instance.bound_closure = make_shared<Environment>(closure);
	method_instance.closure = method_instance.bound_closure.get();
	method_instance.bound_closure->define("this", instance->shared_from_this());
	// have to keep it consistent
	method_instance.is_initializer = is_initializer;

	return method_instance;
}

LoxObject LoxFunction::operator()(Interpreter &it, const vector<LoxObject> &args) {
	auto env = make_unique<Environment>(closure);
	for (size_t i=0; i<declaration->params.size(); i++) {
		env->define(declaration->params[i].lexeme, args[i]);
	}
	try {
		it.execute_block(declaration->body, env.get());
	}
	catch (ReturnUnwind &res) {
		if (is_initializer) {
			return closure->get_at(0, "this");
		}
		return res.value;
	}
	if (is_initializer) {
		return closure->get_at(0, "this");
	}
	return std::monostate{};
}

bool LoxFunction::operator==(const LoxFunction &) {
	return false;
}

bool operator==(const LoxFunction &, const LoxFunction &) {
	return false;
}

NativeFunction::NativeFunction(size_t arity,
	function<LoxObject(Interpreter &, const vector<LoxObject> &)> f): arity(arity), f(f) {
}

LoxObject NativeFunction::operator()(Interpreter &it, const vector<LoxObject> &args) {
	return f(it, args);
}

bool NativeFunction::operator==(const NativeFunction &) {
	return false;
}

bool operator==(const NativeFunction &, const NativeFunction &) {
	return false;
}


LoxClass::LoxClass(string_view name, shared_ptr<LoxClass> superclass, unordered_map<string, LoxFunction> methods):
	name(name), superclass(superclass), methods(std::move(methods)) {
	auto initializer(find_method("init"));
	if (initializer == std::nullopt) {
		arity = 0;
	}
	else {
		arity = initializer.value().arity;
	}
}

LoxInstance LoxClass::operator()(Interpreter &it, const std::vector<LoxObject> &args) {
	LoxInstance instance(shared_from_this());
	auto initializer(find_method("init"));
	if (initializer != std::nullopt) {
		initializer.value().bind(&instance)(it, args);
	}
	return LoxInstance(shared_from_this());
}

std::optional<LoxFunction> LoxClass::find_method(const string &method_name) {
	if (methods.contains(method_name)) {
		return methods.at(method_name);
	}
	if (superclass != nullptr) {
		return superclass->find_method(method_name);
	}
	return std::nullopt;
}


// temporary
bool LoxClass::operator==(const LoxClass &) {
	return false;
}
bool operator==(const LoxClass &, const LoxClass &) {
	return false;
}


LoxInstance::LoxInstance(shared_ptr<LoxClass> klass): klass(std::move(klass)) { }

LoxObject LoxInstance::get(const Token &name) {
	if (fields.contains(name.lexeme)) {
		return fields[name.lexeme];
	}
	optional<LoxFunction> opt_method = klass->find_method(name.lexeme);
	if (opt_method.has_value()) {
		return opt_method.value().bind(this);
	}

	throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(const Token &name, const LoxObject &value) {
	fields[name.lexeme] = value;
}

// temporary
bool LoxInstance::operator==(const LoxInstance &) {
	return false;
}
bool operator==(const LoxInstance &, const LoxInstance &) {
	return false;
}


struct LoxObjToString {
	string operator()(std::monostate) const { return "nil"; }
	string operator()(int i) const { return std::to_string(i); }
	string operator()(double d) const { return std::to_string(d); }
	string operator()(bool b) const { return b ? "true" : "false"; }
	string operator()(const string &s) const { return s; }
	string operator()(const LoxFunction &f) { return "<fn " + f.declaration->name.lexeme + ">"; }
	string operator()(const NativeFunction &) { return "<native fn>"; }
	string operator()(const shared_ptr<LoxClass> &c) { return c->name; }
	string operator()(const shared_ptr<LoxInstance> &o) { return o->klass->name + " instance"; }
};

std::string to_string(const LoxObject &o) {
	return std::visit(LoxObjToString(), o);
}

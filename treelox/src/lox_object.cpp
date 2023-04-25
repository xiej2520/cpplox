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
	declaration(declaration), closure(closure), arity(declaration->params.size()) { }

LoxObject LoxFunction::operator()(Interpreter &it, const vector<LoxObject> &args) {
	auto env = make_unique<Environment>(closure);
	for (size_t i=0; i<declaration->params.size(); i++) {
		env->define(declaration->params[i].lexeme, args[i]);
	}
	try {
		it.execute_block(declaration->body, env.get());
	}
	catch (ReturnUnwind &res) {
		return res.value;
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


LoxClass::LoxClass(string_view name, unordered_map<string, LoxFunction> methods):
	name(name), methods(std::move(methods)) {}

LoxInstance LoxClass::operator()(Interpreter &, const std::vector<LoxObject> &) {
	return LoxInstance(shared_from_this());
}

std::optional<LoxFunction> LoxClass::find_method(const string &method_name) {
	if (methods.contains(method_name)) {
		return methods.at(method_name);
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


LoxInstance::LoxInstance(shared_ptr<LoxClass> klass): klass(klass) { }

LoxObject LoxInstance::get(const Token &name) {
	if (fields.contains(name.lexeme)) {
		return fields[name.lexeme];
	}
	optional<LoxFunction> method = klass->find_method(name.lexeme);
	if (method.has_value()) {
		return method.value();
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
	string operator()(LoxFunction f) { return "<fn " + f.declaration->name.lexeme + ">"; }
	string operator()(NativeFunction) { return "<native fn>"; }
	string operator()(shared_ptr<LoxClass> c) { return c->name; }
	string operator()(const LoxInstance &o) { return o.klass->name + " instance"; }
};

std::string to_string(const LoxObject &o) {
	return std::visit(LoxObjToString(), o);
}

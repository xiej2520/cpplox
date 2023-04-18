#include "stmt.h"
#include "lox_object.h"
#include "environment.h"
#include "interpreter.h"

using std::function;
using std::make_shared;
using std::vector;

LoxFunction::LoxFunction(std::shared_ptr<Function> declaration): declaration(declaration) {
	arity = declaration->params.size();
}

LoxObject LoxFunction::operator()(Interpreter &it, const vector<LoxObject> &args) {
	auto prev_env = it.environment;
	auto env = make_shared<Environment>(it.globals);
	for (size_t i=0; i<declaration->params.size(); i++) {
		env->define(declaration->params[i].lexeme, args[i]);
	}
	try {
		it.execute_block(declaration->body, env);
	}
	catch (ReturnUnwind &res) {
		it.environment = prev_env; // catching this ReturnUnwind prevents execute_block from resetting the env
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

struct LoxObjToString {
	std::string operator()(std::monostate) const { return "nil"; }
	std::string operator()(int i) const { return std::to_string(i); }
	std::string operator()(double d) const { return std::to_string(d); }
	std::string operator()(bool b) const { return b ? "true" : "false"; }
	std::string operator()(const std::string &s) const { return "\"" + s + "\""; }
	std::string operator()(LoxFunction f) { return "<fn " + f.declaration->name.lexeme + ">"; }
	std::string operator()(NativeFunction) { return "<native fn>"; }
};

std::string to_string(const LoxObject &o) {
	return std::visit(LoxObjToString(), o);
}
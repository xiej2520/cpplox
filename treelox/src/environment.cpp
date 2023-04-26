#include "environment.h"

using std::shared_ptr;

Environment::Environment(): enclosing(nullptr) { }

Environment::Environment(shared_ptr<Environment> enclosing): enclosing(enclosing) { }

void Environment::define(const std::string &name, LoxObject value) {
	values[name] = value;
}

Environment &Environment::ancestor(int distance) {
	Environment *env = this;
	for (int i=0; i<distance; i++) {
		env = env->enclosing.get();
	}
	return *env;
}

LoxObject Environment::get(Token name) {
	if (values.contains(name.lexeme)) {
		return values[name.lexeme];
	}
	if (enclosing != nullptr) {
		return enclosing->get(name);
	}
	std::string errMsg("Undefined variable '" + name.lexeme + "'.");
	throw RuntimeError(name, errMsg);
}

LoxObject Environment::get_at(int distance, std::string name) {
	return ancestor(distance).values[name];
}

void Environment::assign(Token name, LoxObject value) {
	if (values.contains(name.lexeme)) {
		values[name.lexeme] = value;
	}
	else if (enclosing != nullptr) {
		enclosing->assign(name, value);
	}
	else {
		std::string errMsg("Undefined variable '" + name.lexeme + "'.");
		throw RuntimeError(name, errMsg);
	}
}

void Environment::assign_at(int distance, Token name, LoxObject value) {
	ancestor(distance).values[name.lexeme] = value;
}

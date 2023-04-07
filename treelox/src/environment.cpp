#include "environment.h"

Environment::Environment() {
	parent = nullptr;
}

Environment::Environment(Environment &enclosing) {
	parent = &enclosing;
}

void Environment::define(const std::string &name, LoxObject value) {
	values[name] = value;
}

void Environment::define_uninitialized(const std::string &name) {
	uninitialized_values.insert(name);
}

LoxObject Environment::get(Token name) {
	if (values.contains(name.lexeme)) {
		return values[name.lexeme];
	}
	if (uninitialized_values.contains(name.lexeme)) {
		std::string errMsg("Uninitialized variable '" + name.lexeme + "'.");
		throw RuntimeError(name, errMsg);
	}
	if (parent != nullptr) {
		return parent->get(name);
	}
	std::string errMsg("Undefined variable '" + name.lexeme + "'.");
	throw RuntimeError(name, errMsg);
}

void Environment::assign(Token name, LoxObject value) {
	if (values.contains(name.lexeme)) {
		values[name.lexeme] = value;
	}
	else if (uninitialized_values.contains(name.lexeme)) {
		uninitialized_values.erase(name.lexeme);
		values[name.lexeme] = value;
	}
	else if (parent != nullptr) {
		parent->assign(name, value);
	}
	else {
		std::string errMsg("Undefined variable '" + name.lexeme + "'.");
		throw RuntimeError(name, errMsg);
	}
}

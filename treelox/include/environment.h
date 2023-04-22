#pragma once
#include <iostream>

#include <unordered_map>
#include <unordered_set>
#include <optional>

#include <memory>
#include <variant>

#include "lox_object.h"
#include "runtime_error.h"

struct Environment {
	Environment *enclosing; // NON-OWNING raw pointers
	std::unordered_map<std::string, LoxObject> values;
	std::unordered_set<std::string> uninitialized_values;
	
	Environment();
	Environment(Environment *enclosing);

	void define(const std::string &name, LoxObject value);
	void define_uninitialized(const std::string &name);
	Environment &ancestor(int distance);
	LoxObject get(Token name);
	LoxObject get_at(int distance, std::string name);
	void assign(Token name, LoxObject value);
	void assign_at(int distance, Token name, LoxObject value);
};

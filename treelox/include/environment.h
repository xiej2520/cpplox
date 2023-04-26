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
	std::shared_ptr<Environment> enclosing;
	std::unordered_map<std::string, LoxObject> values;

	Environment();
	Environment(std::shared_ptr<Environment> enclosing);

	void define(const std::string &name, LoxObject value);
	Environment &ancestor(int distance);
	LoxObject get(Token name);
	LoxObject get_at(int distance, std::string name);
	void assign(Token name, LoxObject value);
	void assign_at(int distance, Token name, LoxObject value);
};

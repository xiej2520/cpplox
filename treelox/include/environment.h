#pragma once
#include <iostream>

#include <unordered_map>
#include <unordered_set>
#include <optional>

#include <memory>
#include <variant>

#include "runtime_error.h"

struct LoxFunction;

using LoxObject = std::variant<std::monostate, int, double, bool, std::string,
	std::shared_ptr<LoxFunction>>;

struct Environment {
	Environment *parent;
	std::unordered_map<std::string, LoxObject> values;
	std::unordered_set<std::string> uninitialized_values;
	
	Environment();
	Environment(Environment &parent);

	void define(const std::string &name, LoxObject value);
	void define_uninitialized(const std::string &name);
	LoxObject get(Token name);
	void assign(Token name, LoxObject value);
};

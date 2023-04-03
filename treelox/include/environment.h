#pragma once
#include <iostream>

#include <unordered_map>
#include <unordered_set>
#include <optional>
#include "expr.h"
#include "runtime_error.h"

struct Environment {
	Environment *parent;
	std::unordered_map<std::string, LiteralVar> values;
	std::unordered_set<std::string> uninitialized_values;
	
	Environment();
	Environment(Environment &parent);

	void define(const std::string &name, LiteralVar value);
	void define_uninitialized(const std::string &name);
	LiteralVar get(Token name);
	void assign(Token name, LiteralVar value);
};

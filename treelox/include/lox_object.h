#pragma once

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

struct LoxFunction;

using LoxObject = std::variant<std::monostate, int, double, bool, std::string,
	std::shared_ptr<LoxFunction>
// LoxClass
// NativeFunction
// LoxInstance
>;

struct Assign;
struct Binary;
struct Call;
struct Grouping;
struct Literal;
struct Logical;
struct Unary;
struct Variable;

using Expr = std::variant
<
	std::monostate,
	Assign,
	Binary,
	Call,
	Grouping,
	Literal,
	Logical,
	Unary,
	Variable
>;

struct LoxObjToString {
	std::string operator()(std::monostate m) const { return "nil"; }
	std::string operator()(int i) const { return std::to_string(i); }
	std::string operator()(double d) const { return std::to_string(d); }
	std::string operator()(bool b) const { return b ? "true" : "false"; }
	std::string operator()(const std::string &s) const { return "\"" + s + "\""; }
	std::string operator()(std::shared_ptr<LoxFunction> f) { return "Function"; }
};

inline std::string to_string(const LoxObject &o) {
	return std::visit(LoxObjToString(), o);
}

class Interpreter;

struct LoxFunction {
	size_t arity;
	LoxFunction(size_t arity, std::function<LoxObject(Interpreter &it, const std::vector<LoxObject> &args)> f);

	std::function<LoxObject(Interpreter &it, const std::vector<LoxObject> &args)> f;
	LoxObject operator()(Interpreter &it, const std::vector<LoxObject> &args);
};

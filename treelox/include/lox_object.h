#pragma once

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

struct LoxFunction;
struct NativeFunction;

using LoxObject = std::variant<std::monostate, int, double, bool, std::string,
	LoxFunction,
	NativeFunction
// LoxClass
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

struct Function;
class Interpreter;

struct LoxFunction {
	std::shared_ptr<Function> declaration;
	// cannot use reference because deleted assign, cannot use weak_ptr because deleted ==
	Interpreter *it;
	size_t arity;
	LoxFunction(std::shared_ptr<Function> declaration, Interpreter *it);

	LoxObject operator()(Interpreter &it, const std::vector<LoxObject> &args);
	bool operator==(const LoxFunction &lf);
	friend bool operator==(const LoxFunction &lf1, const LoxFunction &lf2);
};

struct NativeFunction {
	size_t arity;
	NativeFunction(size_t arity,
		std::function<LoxObject(Interpreter &, const std::vector<LoxObject> &)> f);

	std::function<LoxObject(Interpreter &, const std::vector<LoxObject> &)> f;
	LoxObject operator()(Interpreter &it, const std::vector<LoxObject> &args);

	bool operator==(const NativeFunction &nf);
	friend bool operator==(const NativeFunction &nf1, const NativeFunction &nf2);
};

struct LoxObjToString {
	std::string operator()(std::monostate m) const { return "nil"; }
	std::string operator()(int i) const { return std::to_string(i); }
	std::string operator()(double d) const { return std::to_string(d); }
	std::string operator()(bool b) const { return b ? "true" : "false"; }
	std::string operator()(const std::string &s) const { return "\"" + s + "\""; }
	std::string operator()(LoxFunction f) { return "Function"; }
	std::string operator()(NativeFunction f) { return "Native Function"; }
};

inline std::string to_string(const LoxObject &o) {
	return std::visit(LoxObjToString(), o);
}

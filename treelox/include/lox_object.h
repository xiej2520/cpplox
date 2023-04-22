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
struct Environment;
class Interpreter;

struct LoxFunction {
	const Function *declaration;
	Environment *closure;
	// lifetime of closure needs to at least as long as LoxFunction - I think this works?
	size_t arity;
	LoxFunction(const Function *declaration, Environment *closure);

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

std::string to_string(const LoxObject &o);

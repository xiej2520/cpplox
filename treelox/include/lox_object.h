#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

struct LoxFunction;
struct NativeFunction;
struct LoxClass;
struct LoxInstance;

// try to keep LoxObject size at 16 bytes

using LoxObject = std::variant<std::monostate, int, double, bool, std::string,
	LoxFunction,
	NativeFunction,
	LoxClass,
	LoxInstance
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
	const Function *declaration; // non-owning
	// lifetime of closure needs to at least as long as LoxFunction - I think this works?
	Environment *closure;
	size_t arity;
	LoxFunction(const Function *declaration, Environment *closure);

	LoxObject operator()(Interpreter &it, const std::vector<LoxObject> &args);
	bool operator==(const LoxFunction &);
	friend bool operator==(const LoxFunction &, const LoxFunction &);
};

struct NativeFunction {
	size_t arity;
	NativeFunction(size_t arity,
		std::function<LoxObject(Interpreter &, const std::vector<LoxObject> &)> f);

	std::function<LoxObject(Interpreter &, const std::vector<LoxObject> &)> f;
	LoxObject operator()(Interpreter &it, const std::vector<LoxObject> &args);

	bool operator==(const NativeFunction &);
	friend bool operator==(const NativeFunction &, const NativeFunction &);
};

struct LoxClass {
	std::string name;
	std::unordered_map<std::string, LoxFunction> methods;

	LoxClass(std::string_view name, std::unordered_map<std::string, LoxFunction> methods);
	LoxInstance operator()(Interpreter &it, const std::vector<LoxObject> &args);
	
	std::optional<LoxFunction> find_method(const std::string &method_name);	

	bool operator==(const LoxClass &);
	friend bool operator==(const LoxClass &, const LoxClass &);
};

struct Token;

struct LoxInstance {
	LoxClass *klass; // non-owning
	std::unordered_map<std::string, LoxObject> fields;
	LoxInstance(LoxClass &klass);

	LoxObject get(const Token &name);
	void set(const Token &name, const LoxObject &value);

	bool operator==(const LoxInstance &);
	friend bool operator==(const LoxInstance &, const LoxInstance &);
};

std::string to_string(const LoxObject &o);

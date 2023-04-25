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
	std::shared_ptr<LoxClass>,   // classes need to be kept alive after they're reassigned
	std::shared_ptr<LoxInstance> // LoxInstances are passed and assigned by reference
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
	// for methods, need to carry environment where 'this' is bound to object
	std::shared_ptr<Environment> bound_closure;
	size_t arity;
	bool is_initializer = false;
	LoxFunction(const Function *declaration, Environment *closure);
	LoxFunction bind(LoxInstance *instance);

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

/**
 * LoxClass needs to be kept alive as long as there are instances of it - so it
 * will be kept as shared_ptr
 * inherit enable_shared_from_this to allow operator() to make shared_ptr from itself
 */
struct LoxClass: std::enable_shared_from_this<LoxClass> {
	std::string name;
	std::shared_ptr<LoxClass> superclass;
	std::shared_ptr<Environment> super_closure;
	std::unordered_map<std::string, LoxFunction> methods;
	size_t arity;

	LoxClass(std::string_view name, std::shared_ptr<LoxClass> superclass, std::unordered_map<std::string, LoxFunction> methods);
	LoxInstance operator()(Interpreter &it, const std::vector<LoxObject> &args);
	
	std::optional<LoxFunction> find_method(const std::string &method_name);	

	bool operator==(const LoxClass &);
	friend bool operator==(const LoxClass &, const LoxClass &);
};

struct Token;

/**
 * LoxInstances are passed by reference, therefore use shared_ptr
 * to track LoxInstances.
*/
struct LoxInstance: std::enable_shared_from_this<LoxInstance> {
	std::shared_ptr<LoxClass> klass;
	std::unordered_map<std::string, LoxObject> fields;
	LoxInstance(std::shared_ptr<LoxClass> klass);

	LoxObject get(const Token &name);
	void set(const Token &name, const LoxObject &value);

	bool operator==(const LoxInstance &);
	friend bool operator==(const LoxInstance &, const LoxInstance &);
};

std::string to_string(const LoxObject &o);

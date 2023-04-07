#include "lox_object.h"

struct Environment;

LoxFunction::LoxFunction(
	size_t arity,
	std::function<LoxObject(Interpreter &it, const std::vector<LoxObject> &args)> f):
	arity(arity), f(f) { }

LoxObject LoxFunction::operator()(Interpreter &it, const std::vector<LoxObject> &args) {
	return f(it, args);	
}

#include "stmt.h"
#include "lox_object.h"

LoxFunction::LoxFunction(std::shared_ptr<Function> declaration, Interpreter *it): declaration(declaration), it(it) {
	arity = declaration->params.size();
}

LoxObject LoxFunction::operator()(Interpreter &it, const std::vector<LoxObject> &args) {
	return std::monostate{};
}

bool LoxFunction::operator==(const LoxFunction &lf) {
	return false;
}

bool operator==(const LoxFunction &lf1, const LoxFunction &lf2) {
	return false;
}

NativeFunction::NativeFunction(size_t arity,
	std::function<LoxObject(Interpreter &, const std::vector<LoxObject> &)> f): arity(arity), f(f) {
}

LoxObject NativeFunction::operator()(Interpreter &it, const std::vector<LoxObject> &args) {
	return f(it, args);
}

bool NativeFunction::operator==(const NativeFunction &nf) {
	return false;
}

bool operator==(const NativeFunction &nf1, const NativeFunction &nf2) {
	return false;
}
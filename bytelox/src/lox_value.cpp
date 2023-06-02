#include "lox_value.hpp"
#include "lox_object.hpp"

#define FMT_HEADER_ONLY
#include "fmt/core.h"

namespace bytelox {

void LoxObject::print_object() {
	switch (type) {
		case ObjectType::STRING: fmt::print("{}", as_string().chars.get()); return;
		case ObjectType::UPVALUE: fmt::print("upvalue"); return;
		case ObjectType::FUNCTION: {
			if (as_function().name == nullptr) fmt::print("<script>");
			else fmt::print("<fn {}>", as_function().name->chars.get());
			return;
		}
		case ObjectType::NATIVE: {
			fmt::print("<native fn>"); return;
		}
		case ObjectType::CLOSURE: {
			if (as_closure().function->name == nullptr) fmt::print("<script>");
			else fmt::print("<fn {}>", as_closure().function->name->chars.get());
			return;
		}
		case ObjectType::CLASS: {
			fmt::print("{}", as_class().name->chars.get());
			break;
		}
		case ObjectType::INSTANCE: {
			fmt::print("{} instance", as_instance().klass->name->chars.get());
		}
	}

}

void LoxValue::print_value() {
	switch (type) {
		case ValueType::BOOL:
			fmt::print("{}", as.boolean);
			break;
		case ValueType::NIL: fmt::print("nil"); break;
		case ValueType::NUMBER: fmt::print("{:g}", as.number); break;
		case ValueType::OBJECT: as.obj->print_object(); break;
	}
}

}

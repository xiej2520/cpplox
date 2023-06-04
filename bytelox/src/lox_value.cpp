#include "lox_value.hpp"
#include "lox_object.hpp"

#define FMT_HEADER_ONLY
#include "fmt/core.h"

namespace bytelox {

bool LoxValue::is_string() {
	return is_object() && as.obj->is_string();
}

bool LoxValue::is_upvalue() {
	return is_object() && as.obj->is_upvalue();
}

bool LoxValue::is_function() {
	return is_object() && as.obj->is_function();
}

bool LoxValue::is_native() {
	return is_object() && as.obj->is_native();
}

bool LoxValue::is_closure() {
	return is_object() && as.obj->is_closure();
}

bool LoxValue::is_class() {
	return is_object() && as.obj->is_class();
}

bool LoxValue::is_instance() {
	return is_object() && as.obj->is_instance();
}

bool LoxValue::is_bound_method() {
	return is_object() && as.obj->is_bound_method();
}

ObjectString &LoxValue::as_string() {
	return static_cast<ObjectString &>(*as.obj);
}

ObjectUpvalue &LoxValue::as_upvalue() {
	return static_cast<ObjectUpvalue &>(*as.obj);
}

ObjectFunction &LoxValue::as_function() {
	return static_cast<ObjectFunction &>(*as.obj);
}

ObjectNative &LoxValue::as_native() {
	return static_cast<ObjectNative &>(*as.obj);
}

ObjectClass &LoxValue::as_class() {
	return static_cast<ObjectClass &>(*as.obj);
}

ObjectClosure &LoxValue::as_closure() {
	return static_cast<ObjectClosure &>(*as.obj);
}

ObjectInstance &LoxValue::as_instance() {
	return static_cast<ObjectInstance &>(*as.obj);
}

ObjectBoundMethod &LoxValue::as_bound_method() {
	return static_cast<ObjectBoundMethod &>(*as.obj);
}

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
			break;
		}
		case ObjectType::BOUND_METHOD: {
			if (as_bound_method().method->function == nullptr) fmt::print("<script>");
			else fmt::print("<fn {}>", as_bound_method().method->function->name->chars.get());
			break;
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

#pragma once

#include <memory>
#include <cstring>
#include <string>

#include "common.hpp"

namespace bytelox {

enum class ValueType {
	BOOL,
	NIL,
	NUMBER,
	OBJECT,
};

struct LoxObject;
struct ObjectString;
struct ObjectUpvalue;
struct ObjectFunction;
struct ObjectNative;
struct ObjectClosure;
struct ObjectClass;
struct ObjectInstance;
struct ObjectBoundMethod;

#undef NAN_BOXING
#ifdef NAN_BOXING

#else

struct LoxValue {
	ValueType type;
	union {
		bool boolean;
		double number;
		LoxObject *obj;
	} as;
	
	// definition needs to be provided
	// implicit conversions allowed
	constexpr LoxValue(): type(ValueType::NIL), as({ .number=0 }) { }
	constexpr LoxValue(bool b): type(ValueType::BOOL), as({ .boolean=b }) { }
	constexpr LoxValue(double d): type(ValueType::NUMBER), as({ .number=d }) { }
	constexpr LoxValue(LoxObject *obj): type(ValueType::OBJECT), as({ .obj=obj }) { }
	
	[[nodiscard]] constexpr bool is_bool() const {
		return type == ValueType::BOOL;
	}
	[[nodiscard]] constexpr bool is_nil() const {
		return type == ValueType::NIL;
	};
	[[nodiscard]] constexpr bool is_number() const {
		return type == ValueType::NUMBER;
	};
	[[nodiscard]] constexpr bool is_object() const {
		return type == ValueType::OBJECT;
	};
	
	bool is_string();
	bool is_upvalue();
	bool is_function();
	bool is_native();
	bool is_closure();
	bool is_class();
	bool is_instance();
	bool is_bound_method();
	
	ObjectString   &as_string();
	ObjectUpvalue  &as_upvalue();
	ObjectFunction &as_function();
	ObjectNative   &as_native();
	ObjectClosure  &as_closure();
	ObjectClass    &as_class();
	ObjectInstance &as_instance();
	ObjectBoundMethod &as_bound_method();
	
	[[nodiscard]] constexpr bool operator==(LoxValue that) const {
		if (type != that.type) {
			return false;
		}
		switch (type) {
			case ValueType::BOOL: return as.boolean == that.as.boolean;
			case ValueType::NIL: return true;
			case ValueType::NUMBER: return as.number == that.as.number;
			case ValueType::OBJECT: return as.obj == that.as.obj;
			default: return false; // unreachable
		}
	}

	void print_value();
};
#endif

}

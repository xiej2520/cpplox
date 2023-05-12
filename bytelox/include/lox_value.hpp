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
	OBJECT
};

enum class ObjectType {
	STRING,
};

struct ObjectString;

// inheritance would probably work instead
struct LoxObject {
	ObjectType type;
	LoxObject *next;

	[[nodiscard]] constexpr bool is_type(ObjectType type) const {
		return this->type == type;
	}
	constexpr ObjectString *as_string() {
		return (ObjectString *) this;
	}
	
	void print_object();
};

struct ObjectString {
	LoxObject obj;
	int length; // does NOT including trailing '\0'
	std::unique_ptr<char[]> chars;
	constexpr ObjectString(std::string_view str):
		length(str.size()),
		chars(std::make_unique_for_overwrite<char[]>(str.size())) {
			obj.type = ObjectType::STRING;
			std::memcpy(chars.get(), str.data(), str.size());
			chars[length] = '\0';
	}
};

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
	[[nodiscard]] constexpr bool is_string() const {
		return type == ValueType::OBJECT && as.obj->type == ObjectType::STRING;
	};

	[[nodiscard]] ObjectString &as_string() {
		return (ObjectString &) *as.obj;
	}
	
	[[nodiscard]] constexpr bool operator==(LoxValue that) const {
		if (type != that.type) {
			return false;
		}
		switch (type) {
			case ValueType::BOOL: return as.boolean == that.as.boolean;
			case ValueType::NIL: return true;
			case ValueType::NUMBER: return as.number == that.as.number;
			case ValueType::OBJECT: {
				return as.obj->as_string()->length == that.as.obj->as_string()->length &&
					memcmp(as.obj->as_string()->chars.get(), that.as.obj->as_string()->chars.get(),
					as.obj->as_string()->length);
			}
			default: return false; // unreachable
		}
	}

	void print_value();
};

}

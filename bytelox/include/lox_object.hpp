#pragma once

#include "chunk.hpp"
#include "lox_value.hpp"

namespace bytelox {

enum class ObjectType {
	STRING,
	FUNCTION,
	NATIVE,
};

struct ObjectString;
struct ObjectFunction;
struct ObjectNative;

// inheritance would probably work instead
struct LoxObject {
	ObjectType type;
	LoxObject *next;

	[[nodiscard]] constexpr bool is_type(ObjectType type) const {
		return this->type == type;
	}
	ObjectString &as_string() {
		return (ObjectString &) *this;
	}
	ObjectFunction &as_function() {
		return (ObjectFunction &) *this;
	}
	ObjectNative &as_native() {
		return (ObjectNative &) *this;
	}
	bool is_string() {
		return type == ObjectType::STRING;
	}
	bool is_function() {
		return type == ObjectType::FUNCTION;
	}
	bool is_native() {
		return type == ObjectType::NATIVE;
	}
	
	void print_object();
};

struct ObjectString {
	LoxObject obj;
	u32 length; // does NOT including trailing '\0'
	u32 hash;
	std::unique_ptr<char[]> chars;
	constexpr ObjectString(std::string_view str):
		length(str.size()),
		chars(std::make_unique_for_overwrite<char[]>(length + 1)) {
			obj.type = ObjectType::STRING;
			std::memcpy(chars.get(), str.data(), str.size());
			chars[length] = '\0';
			// FNV-1a hash function
			hash = 2166136261u;
			for (u32 i=0; i<length; i++) {
				hash ^= (u8) str[i];
				hash *= 16777619;
			}
	}
};

struct ObjectFunction {
	LoxObject obj;
	int arity;
	Chunk chunk;
	ObjectString *name;
	constexpr ObjectFunction() {
		obj.type = ObjectType::FUNCTION;
		arity = 0;
		name = nullptr;
	}
};

using NativeFn = LoxValue (*)(int arg_count, LoxValue *args);

struct ObjectNative {
	LoxObject obj;
	NativeFn function;
	constexpr ObjectNative(NativeFn fn): function(fn) {
		obj.type = ObjectType::NATIVE;
	}
};

}

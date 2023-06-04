#pragma once

#include "chunk.hpp"
#include "lox_value.hpp"
#include "hash_table.hpp"

#ifdef DEBUG_LOG_GC
#define FMT_HEADER_ONLY
#include "fmt/core.h"
#endif

namespace bytelox {

enum class ObjectType {
	STRING,
	UPVALUE,
	FUNCTION,
	NATIVE,
	CLOSURE,
	CLASS,
	INSTANCE,
	BOUND_METHOD,
};

struct LoxObject {
	ObjectType type;
	bool is_marked = false;
	LoxObject *next;

	[[nodiscard]] constexpr bool is_type(ObjectType type) const {
		return this->type == type;
	}
	ObjectString &as_string() {
		return (ObjectString &) *this;
	}
	ObjectUpvalue &as_upvalue() {
		return (ObjectUpvalue &) *this;
	}
	ObjectFunction &as_function() {
		return (ObjectFunction &) *this;
	}
	ObjectNative &as_native() {
		return (ObjectNative &) *this;
	}
	ObjectClosure &as_closure() {
		return (ObjectClosure &) *this;
	}
	ObjectClass &as_class() {
		return (ObjectClass &) *this;
	}
	ObjectInstance &as_instance() {
		return (ObjectInstance &) *this;
	}
	ObjectBoundMethod &as_bound_method() {
		return (ObjectBoundMethod &) *this;
	}
	constexpr bool is_string() {
		return type == ObjectType::STRING;
	}
	constexpr bool is_upvalue() {
		return type == ObjectType::UPVALUE;
	}
	constexpr bool is_function() {
		return type == ObjectType::FUNCTION;
	}
	constexpr bool is_native() {
		return type == ObjectType::NATIVE;
	}
	constexpr bool is_closure() {
		return type == ObjectType::CLOSURE;
	}
	constexpr bool is_class() {
		return type == ObjectType::CLASS;
	}
	constexpr bool is_instance() {
		return type == ObjectType::INSTANCE;
	}
	constexpr bool is_bound_method() {
		return type == ObjectType::BOUND_METHOD;
	}
	
	void print_object();
};

struct ObjectString: LoxObject {
	u32 length; // does NOT including trailing '\0'
	u32 hash;
	std::unique_ptr<char[]> chars;
	constexpr ObjectString(std::string_view str):
		length(str.size()),
		chars(std::make_unique_for_overwrite<char[]>(length + 1)) {
			type = ObjectType::STRING;
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

struct ObjectUpvalue: LoxObject {
	LoxValue *location;
	LoxValue closed = LoxValue();
	ObjectUpvalue *next = nullptr;
	constexpr ObjectUpvalue(LoxValue *slot): location(slot) {
		type = ObjectType::UPVALUE;
	}
};

struct ObjectFunction: LoxObject {
	int arity = 0;
	Chunk chunk;
	ObjectString *name = nullptr;
	int upvalue_count = 0;
	constexpr ObjectFunction() {
		type = ObjectType::FUNCTION;
	}
};

using NativeFn = LoxValue (*)(int arg_count, LoxValue *args);

struct ObjectNative: LoxObject {
	NativeFn function;
	constexpr ObjectNative(NativeFn fn): function(fn) {
		type = ObjectType::NATIVE;
	}
};

struct ObjectClosure: LoxObject {
	ObjectFunction *function;
	ObjectUpvalue **upvalues;
	int upvalue_count;
	ObjectClosure(ObjectFunction *fn): function(fn),
			upvalues(new ObjectUpvalue *[fn->upvalue_count]()),
			upvalue_count(fn->upvalue_count) {
#ifdef DEBUG_LOG_GC
	fmt::print("{} allocate {} for {}\n", (void *) upvalues, sizeof(ObjectUpvalue), "ObjectUpvalue");
#endif
		type = ObjectType::CLOSURE;
	}
};

struct ObjectClass: LoxObject {
	ObjectString *name;
	HashTable methods;
	ObjectClass(ObjectString *str): name(str) {
		type = ObjectType::CLASS;
	}
};

struct ObjectInstance: LoxObject {
	ObjectClass *klass;
	HashTable fields;
	ObjectInstance(ObjectClass *klass): klass(klass) {
		type = ObjectType::INSTANCE;
	}
};

struct ObjectBoundMethod: LoxObject {
	LoxValue receiver;
	ObjectClosure *method;
	ObjectBoundMethod(LoxValue receiver, ObjectClosure *method):
			receiver(receiver), method(method) {
		type = ObjectType::BOUND_METHOD;
	}
};

}

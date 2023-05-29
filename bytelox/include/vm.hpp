#pragma once

#include "chunk.hpp"
#include "hash_table.hpp"

#include <string>
#include <vector>

#define FMT_HEADER_ONLY
#include "fmt/core.h"

namespace bytelox {

enum class InterpretResult {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
};

struct VM {
	struct CallFrame {
		ObjectFunction *function;
		u8 *ip;
		size_t slots; // start of frame slots in stack
		CallFrame(ObjectFunction* fn, u8 *ip, size_t slots): function(fn), ip(ip), slots(slots) {}
	};

	Chunk *chunk;
	u8 *ip = nullptr; // next instruction to be executed
	std::vector<LoxValue> stack;
	LoxObject *objects;
	HashTable globals;
	HashTable strings;
	
	std::vector<CallFrame> frames;

	VM();
	~VM();
	VM(VM &vm) = delete;
	VM &operator=(VM &vm) = delete;
	
	// returns the ith element from the top of the stack, 0-indexed. No bounds check
	LoxValue &peek(size_t i);
	// top of stack, no bounds check
	LoxValue &peek();
	bool call(ObjectFunction *fn, int arg_count);
	bool call_value(LoxValue callee, int arg_count);
	LoxValue read_constant(CallFrame *frame);
	void concatenate();
	
	LoxValue make_LoxObject(LoxObject *obj);
	LoxValue make_ObjectString(std::string_view str);
	LoxValue make_ObjectFunction(ObjectFunction *fn);
	LoxValue make_ObjectNative(NativeFn function);
	void free_LoxObject(LoxObject *object);
	void define_native(std::string_view name, NativeFn fn);
	
	InterpretResult interpret(std::string_view src);
	InterpretResult run();
	
	template<typename... Args>
	void runtime_error(fmt::format_string<Args...> format, Args&&... args);
	void reset_stack();

};

}

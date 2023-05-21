#pragma once

#include "chunk.hpp"
#include "hash_table.hpp"

#include <string>
#include <vector>

#include "fmt/core.h"

namespace bytelox {

enum class InterpretResult {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
};

struct VM {
	Chunk *chunk;
	u8 *ip = nullptr; // next instruction to be executed
	std::vector<LoxValue> stack;
	LoxObject *objects;
	HashTable strings;

	VM();
	~VM();
	VM(VM &vm) = delete;
	VM &operator=(VM &vm) = delete;
	
	// returns the ith element from the top of the stack, 0-indexed. No bounds check
	LoxValue &peek(size_t i);
	// top of stack, no bounds check
	LoxValue &peek();
	void concatenate();
	
	LoxValue make_LoxObject(LoxObject *obj);
	LoxValue make_ObjectString(std::string_view str);
	void free_LoxObject(LoxObject *object);
	
	InterpretResult interpret(std::string_view src);
	InterpretResult run();
	
	template<typename... Args>
	void runtime_error(fmt::format_string<Args...> format, Args&&... args);
	void reset_stack();

};

}

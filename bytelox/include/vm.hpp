#pragma once

#include "chunk.hpp"
#include "hash_table.hpp"
#include "compiler.hpp"

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
		ObjectClosure *closure;
		u8 *ip;
		size_t slots; // index of frame (fn obj) in stack
		CallFrame(ObjectClosure* closure, u8 *ip, size_t slots):
				closure(closure), ip(ip), slots(slots) {}
	};

	Compiler *compiler;
	Chunk *chunk;
	u8 *ip = nullptr; // next instruction to be executed
	std::vector<LoxValue> stack;

	LoxObject *objects = nullptr;
	size_t bytes_allocated = 0;
	size_t next_GC = 1024 * 1024;
	
	
	ObjectUpvalue *open_upvalues = nullptr;
	HashTable globals;
	HashTable strings;
	
	std::vector<CallFrame> frames;
	std::vector<LoxObject *> gray_stack;
	
	ObjectString *init_string = nullptr;

	VM();
	~VM();
	VM(VM &vm) = delete;
	VM &operator=(VM &vm) = delete;
	
	// returns the ith element from the top of the stack, 0-indexed. No bounds check
	LoxValue &peek(size_t i);
	// top of stack, no bounds check
	LoxValue &peek();
	bool call(ObjectClosure &closure, int arg_count);
	bool call_value(LoxValue callee, int arg_count);
	ObjectUpvalue *capture_upvalue(LoxValue *local);
	void close_upvalues(LoxValue *last);
	void define_method(ObjectString *name);
	bool bind_method(ObjectClass *klass, ObjectString *name);
	bool invoke(ObjectString *name, int arg_count);
	bool invoke_from_class(ObjectClass *klass, ObjectString *name, int arg_count);
	LoxValue read_constant(CallFrame *frame);
	void concatenate();
	
	LoxValue make_LoxObject(LoxObject *obj);
	LoxValue make_ObjectString(std::string_view str);
	LoxValue make_ObjectFunction(ObjectFunction *fn);
	LoxValue make_ObjectNative(NativeFn function);
	LoxValue make_ObjectClosure(ObjectClosure *closure);
	LoxValue make_ObjectClass(ObjectString *str);
	LoxValue make_ObjectInstance(ObjectClass *klass);
	LoxValue make_ObjectBoundMethod(LoxValue receiver, ObjectClosure *method);
	void free_LoxObject(LoxObject *object);
	void define_native(std::string_view name, NativeFn fn);
	
	InterpretResult interpret(std::string_view src);
	InterpretResult run();
	
	template<typename... Args>
	void runtime_error(fmt::format_string<Args...> format, Args&&... args);
	void reset_stack();
	
	void mark_roots();
	void mark_compiler_roots();
	void mark_value(LoxValue &val);
	void mark_object(LoxObject *obj);
	void mark_vec(std::vector<LoxValue> &vec);
	void mark_table(HashTable &table);
	void remove_white(HashTable &table);
	void blacken_object(LoxObject &obj);
	void collect_garbage();
	void trace_references();
	void sweep();

};

}

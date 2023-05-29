#include "common.hpp"
#include "lox_object.hpp"
#include "lox_value.hpp"
#include "vm.hpp"
#include "debug.hpp"
#include "compiler.hpp"

namespace bytelox {

#include <time.h>
LoxValue clock_native(int, LoxValue *) {
	return LoxValue((double) clock() / CLOCKS_PER_SEC);
}

using enum InterpretResult;

VM::VM(): objects(nullptr) {
	define_native("clock", clock_native);
}

VM::~VM() {
	while (objects != nullptr) {
		LoxObject *next = objects->next;
		free_LoxObject(objects);
		objects = next;
	}
}

InterpretResult VM::interpret(std::string_view src) {
	Scanner scanner(src);
	Compiler compiler(scanner, *this);
	ObjectFunction *fn = compiler.compile(src);
	if (fn == nullptr) return INTERPRET_COMPILE_ERROR;
	
	stack.push_back(make_ObjectFunction(fn));
	frames.emplace_back(fn, fn->chunk.code.data(), 0);

	return run();
}

LoxValue VM::make_LoxObject(LoxObject *obj) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	if (obj == nullptr) {
		res.as.obj = new LoxObject;
	}
	else {
		res.as = { .obj=obj };
	}
	res.as.obj->next = objects;
	objects = res.as.obj;
	return res;	
}

LoxValue VM::make_ObjectString(std::string_view str) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	ObjectString *interned = strings.find_string(str);
	if (interned == nullptr) {
		res.as.obj = (LoxObject *) new ObjectString(str);
		strings.set((ObjectString *) res.as.obj, LoxValue());
		res.as.obj->next = objects;
		objects = res.as.obj;
	}
	else {
		res.as.obj = (LoxObject *) interned;
	}
	return res;	
}

// end_compiler takes the function from the LocalState, pass it here to create
// it in the VM
LoxValue VM::make_ObjectFunction(ObjectFunction *fn) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	res.as.obj = (LoxObject *) fn;
	res.as.obj->next = objects;
	objects = res.as.obj;
	return res;
}

LoxValue VM::make_ObjectNative(NativeFn function) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	res.as.obj = (LoxObject *) new ObjectNative(function);
	res.as.obj->next = objects;
	objects = res.as.obj;
	return res;
}

void VM::free_LoxObject(LoxObject *object) {
	switch (object->type) {
		case ObjectType::STRING: {
			delete object; // unique_ptr frees chars
			break;
		}
		case ObjectType::FUNCTION: {
			delete object;
			break;
		}
		case ObjectType::NATIVE: {
			delete object;
			break;
		} 
	}
}

void VM::define_native(std::string_view name, NativeFn fn) {
	stack.push_back(make_ObjectString(name));
	stack.push_back(make_ObjectNative(fn));
	globals.set(&stack[0].as.obj->as_string(), stack[1]);
	stack.pop_back();
	stack.pop_back();
}

bool is_falsey(LoxValue value) {
	return value.is_nil() || (value.is_bool() && !value.as.boolean);
}

void VM::concatenate() {
	ObjectString &b = peek().as.obj->as_string();
	ObjectString &a = peek(1).as.obj->as_string();
	int length = a.length + b.length;
	std::unique_ptr<char[]> chars = std::make_unique_for_overwrite<char[]>(length + 1);
	std::memcpy(chars.get(), a.chars.get(), a.length);
	std::memcpy(chars.get() + a.length, b.chars.get(), b.length);
	chars[length] = '\0';
	peek(1).as.obj->as_string().length = length;
	peek(1).as.obj->as_string().chars = std::move(chars);
	stack.pop_back();
}

// inline?
LoxValue &VM::peek(size_t i) {
	return *(stack.rbegin() + i);
}

LoxValue &VM::peek() {
	return stack.back();
}

bool VM::call(ObjectFunction *fn, int arg_count) {
	if (arg_count != fn->arity) {
		runtime_error("Expected {} arguments but got {}.", fn->arity, arg_count);
		return false;
	}
	frames.emplace_back(fn, fn->chunk.code.data(), stack.size() - arg_count);
	return true;
}

bool VM::call_value(LoxValue callee, int arg_count) {
	if (callee.is_object()) {
		switch(callee.as.obj->type) {
			case ObjectType::FUNCTION: return call(&callee.as.obj->as_function(), arg_count);
			case ObjectType::NATIVE: {
				NativeFn native = callee.as.obj->as_native().function;
				LoxValue result = native(arg_count, &stack.back() - arg_count + 1);
				stack.resize(stack.size() - arg_count); // get rid of args (leave first for inplace)
				stack.back() = result;
				return true;
			}
			default: break; // Non-callable object typ
		}
	}
	runtime_error("Can only call functions and classes.");
	return false;
}

LoxValue VM::read_constant(CallFrame *frame) {
	return frame->function->chunk.constants[*frame->ip++];
}

InterpretResult VM::run() {
	for (;;) {
		CallFrame *frame = &frames.back();
#undef DEBUG_TRACE_EXECUTION
#ifdef DEBUG_TRACE_EXECUTION
		fmt::print("          ");
		for (LoxValue value : stack) {
			fmt::print("[ ");
			value.print_value();
			fmt::print((" ]"));
		}
		fmt::print("\n");
		disassemble_instruction(frame->function->chunk, frame->ip - frame->function->chunk.code.data());
#endif
#define READ_BYTE() (*frame->ip++)
		u8 instruction;
		switch (instruction = *frame->ip++) {
		case +OP::CONSTANT: {
			stack.push_back(read_constant(frame));
			break;
		}
		case +OP::CONSTANT_LONG: {
			size_t index = *frame->ip | (*(frame->ip+1) << 8) | (*(frame->ip+2) << 16);
			frame->ip += 3;
			stack.push_back(chunk->constants[index]);
			break;
		}
		case +OP::NIL: stack.emplace_back(LoxValue()); break;
		case +OP::TRUE: stack.emplace_back(LoxValue(true)); break;
		case +OP::FALSE: stack.emplace_back(LoxValue(false)); break;
		case +OP::POP: stack.pop_back(); break;
		case +OP::GET_LOCAL: {
			u8 slot = *frame->ip++ + frame->slots;
			stack.push_back(stack[slot]); // loads local to top of stack
			break;
		}
		case +OP::SET_LOCAL: {
			u8 slot = *frame->ip++ + frame->slots;
			stack[slot] = peek();
			break;
		}
		case +OP::GET_GLOBAL: {
			ObjectString *name = &read_constant(frame).as.obj->as_string();
			LoxValue value;
			if (!globals.get(name, &value)) {
				runtime_error("Undefined variable '{}'.", name->chars.get());
				return INTERPRET_RUNTIME_ERROR;
			}
			stack.push_back(value);
			break;
		}
		case +OP::DEFINE_GLOBAL: {
			ObjectString *name = &read_constant(frame).as.obj->as_string();
			globals.set(name, peek());
			stack.pop_back();
			break;
		}
		case +OP::SET_GLOBAL: {
			ObjectString *name = (ObjectString *) read_constant(frame).as.obj;
			// using set to check if defined?
			if (globals.set(name, peek())) {
				globals.del(name);
				runtime_error("Undefined variable '{}'", name->chars.get());
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case +OP::EQUAL: {
			peek(1) = LoxValue(peek(1) == peek(0));
			stack.pop_back();
			break;
		}
		case +OP::NOT_EQUAL: {
			peek(1) = LoxValue(peek(1) != peek(0));
			stack.pop_back();
			break;
		}
		case +OP::GREATER: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1) = LoxValue(peek(1).as.number > peek().as.number);
			stack.pop_back();
			break;
		}
		case +OP::GREATER_EQUAL: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1) = LoxValue(peek(1).as.number >= peek().as.number);
			stack.pop_back();
			break;
		}
		case +OP::LESS: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1) = LoxValue(peek(1).as.number < peek().as.number);
			stack.pop_back();
			break;
		}
		case +OP::LESS_EQUAL: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1) = LoxValue(peek(1).as.number <= peek().as.number);
			stack.pop_back();
			break;
		}
		case +OP::ADD: {
			if (peek().is_object() && peek().as.obj->is_string() && peek(1).is_object() && peek(1).as.obj->is_string()) {
				concatenate();
			}
			else if (peek().is_number() && peek(1).is_number()) {
				peek(1).as.number += peek().as.number;
				stack.pop_back();
			}
			else {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case +OP::SUB: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1).as.number -= peek().as.number;
			stack.pop_back();
			break;
		}
		case +OP::MUL: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1).as.number *= peek().as.number;
			stack.pop_back();
			break;
		}
		case +OP::DIV: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1).as.number /= peek().as.number;
			stack.pop_back();
			break;
		}
		case +OP::NOT: {
			peek() = is_falsey(peek());
			break;
		}
		case +OP::NEGATE: {
			if (!peek().is_number()) {
				runtime_error("Operand must be a number.");
				return InterpretResult::INTERPRET_RUNTIME_ERROR;
			}
			peek().as.number *= -1;
			break;
		}
		case +OP::PRINT: {
			peek().print_value();
			stack.pop_back();
			break;
		}
		case +OP::JUMP: {
			u16 offset = *frame->ip | (*(frame->ip+1) << 8);
			frame->ip += offset;
			break;
		}
		case +OP::JUMP_IF_FALSE: {
			if (is_falsey(peek())) {
				u16 offset = *frame->ip | (*(frame->ip+1) << 8);
				frame->ip += offset;
				break;
			}
			frame->ip += 2; // past offset
			break;
		}
		case +OP::LOOP: {
			u16 offset = *frame->ip | (*(frame->ip+1) << 8);
			frame->ip -= offset;
			break;
		}
		case +OP::CALL: {
			int arg_count = *frame->ip++;
			if (!call_value(peek(arg_count), arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &frames.back();
			break;
		}
		case +OP::RETURN: {
			LoxValue result = stack.back();
			stack.pop_back(); // get rid of returned value
			if (frames.size() == 1) { // last frame popped
				frames.pop_back();
				stack.pop_back();
				return INTERPRET_OK;
			}
			
			stack.resize(frame->slots - 1); // resize stack down
			stack.push_back(result); // put result at top of stack
			frames.pop_back(); // drop frame
			frame = &frames.back();
			break;
		}
		}
	}
}

template<typename... Args>
void VM::runtime_error(fmt::format_string<Args...> format, Args&&... args) {
	fmt::vprint(stderr, format, fmt::make_format_args(std::forward<Args>(args)...)); // wow
	fmt::print(stderr, "\n");
	for (int i=frames.size()-1; i>=0; i--) {
		CallFrame &frame = frames[i];
		ObjectFunction *fn = frame.function;
		size_t instruction = frame.ip - fn->chunk.code.data() - 1;
		fmt::print(stderr, "[line {}] in ", fn->chunk.get_line(instruction));
		if (fn->name == nullptr) fmt::print(stderr, "script\n");
		else fmt::print(stderr, "{}()\n", fn->name->chars.get());
	}

	CallFrame &frame = frames.back();
	size_t instruction = frame.ip - frame.function->chunk.code.data() - 1; // ip advances before executing
	u16 line = frame.function->chunk.get_line(instruction);
	fmt::print(stderr, "[line {}] in script\n", line);
	reset_stack();
}

void VM::reset_stack() {
	stack.clear();
}

} // namespace bytelox

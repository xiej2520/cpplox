#include "common.hpp"
#include "lox_value.hpp"
#include "vm.hpp"
#include "debug.hpp"
#include "compiler.hpp"

#include "fmt/core.h"

namespace bytelox {

using enum InterpretResult;

VM::VM(): objects(nullptr) { }

VM::~VM() {
	while (objects != nullptr) {
		LoxObject *next = objects->next;
		free_LoxObject(objects);
		objects = next;
	}
}

InterpretResult VM::interpret(std::string_view src) {
	chunk = new Chunk;
	Scanner scanner(src);
	Compiler compiler(scanner, *this);
	if (!compiler.compile(src, *chunk)) {
		delete chunk;
		return INTERPRET_COMPILE_ERROR;
	}
	compiler.end_compiler();
	ip = chunk->code.data(); // instruction pointer
	InterpretResult res = run();
	delete chunk;
	return res;
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
	LoxValue res = make_LoxObject(nullptr);
	delete res.as.obj;
	ObjectString *interned = strings.find_string(str);
	if (interned == nullptr) {
		res.as.obj = (LoxObject *) new ObjectString(str);
		strings.set((ObjectString *) res.as.obj, LoxValue());
	}
	else {
		res.as.obj = (LoxObject *) interned;
	}
	return res;	
}

void VM::free_LoxObject(LoxObject *object) {
	switch (object->type) {
		case ObjectType::STRING: {
			delete object; // unique_ptr frees chars
		}
	}
}

bool is_falsey(LoxValue value) {
	return value.is_nil() || (value.is_bool() && !value.as.boolean);
}

void VM::concatenate() {
	ObjectString &b = peek().as_string();
	ObjectString &a = peek(1).as_string();
	int length = a.length + b.length;
	std::unique_ptr<char[]> chars = std::make_unique_for_overwrite<char[]>(length + 1);
	std::memcpy(chars.get(), a.chars.get(), a.length);
	std::memcpy(chars.get() + a.length, b.chars.get(), b.length);
	chars[length] = '\0';
	peek(1).as_string().length = length;
	peek(1).as_string().chars = std::move(chars);
	stack.pop_back();
}

// inline?
LoxValue &VM::peek(size_t i) {
	return *(stack.rbegin() + i);
}

LoxValue &VM::peek() {
	return stack.back();
}

LoxValue VM::read_constant() {
	return chunk->constants[*ip++];
}

InterpretResult VM::run() {
	for (;;) {
#undef DEBUG_TRACE_EXECUTION
#ifdef DEBUG_TRACE_EXECUTION
		fmt::print("          ");
		for (LoxValue value : stack) {
			fmt::print("[ ");
			value.print_value();
			fmt::print((" ]"));
		}
		fmt::print("\n");
		disassemble_instruction(*chunk, ip - chunk->code.data());
#endif
		u8 instruction;
		switch (instruction = *ip++) {
		case +OP::CONSTANT: {
			stack.push_back(read_constant());
			break;
		}
		case +OP::CONSTANT_LONG: {
			size_t index = *ip | (*(ip+1) << 8) | (*(ip+2) << 8);
			ip += 3;
			stack.push_back(chunk->constants[index]);
			break;
		}
		case +OP::NIL: stack.emplace_back(LoxValue()); break;
		case +OP::TRUE: stack.emplace_back(LoxValue(true)); break;
		case +OP::FALSE: stack.emplace_back(LoxValue(false)); break;
		case +OP::POP: stack.pop_back(); break;
		case +OP::GET_LOCAL: {
			u8 slot = *ip++;
			stack.push_back(stack[slot]); // loads local to top of stack
			break;
		}
		case +OP::SET_LOCAL: {
			u8 slot = *ip++;
			stack[slot] = peek();
			break;
		}
		case +OP::GET_GLOBAL: {
			ObjectString *name = (ObjectString *) read_constant().as.obj;
			LoxValue value;
			if (!globals.get(name, &value)) {
				runtime_error("Undefined variable '{}'.", name->chars.get());
				return INTERPRET_RUNTIME_ERROR;
			}
			stack.push_back(value);
			break;
		}
		case +OP::DEFINE_GLOBAL: {
			ObjectString *name = (ObjectString *) read_constant().as.obj;
			globals.set(name, peek());
			stack.pop_back();
			break;
		}
		case +OP::SET_GLOBAL: {
			ObjectString *name = (ObjectString *) read_constant().as.obj;
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
			if (peek().is_string() && peek(1).is_string()) {
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
			u16 offset = *ip | (*(ip+1) << 8);
			ip += offset;
			break;
		}
		case +OP::JUMP_IF_FALSE: {
			if (is_falsey(peek())) {
				u16 offset = *ip | (*(ip+1) << 8);
				ip += offset;
				break;
			}
			ip += 2; // past offset
			break;
		}
		case +OP::LOOP: {
			u16 offset = *ip | (*(ip+1) << 8);
			ip -= offset;
			break;
		}
		case +OP::RETURN: {
			return INTERPRET_OK;
		}
		}
	}
}

template<typename... Args>
void VM::runtime_error(fmt::format_string<Args...> format, Args&&... args) {
	fmt::vprint(stderr, format, fmt::make_format_args(std::forward<Args>(args)...)); // wow
	fmt::print(stderr, "\n");

	size_t instruction = ip - chunk->code.data() - 1; // ip advances before executing
	u16 line = chunk->get_line(instruction);
	fmt::print(stderr, "[line {}] in script\n", line);
	reset_stack();
}

void VM::reset_stack() {
	stack.clear();
}

} // namespace bytelox

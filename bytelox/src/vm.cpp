#include "common.hpp"
#include "vm.hpp"
#include "debug.hpp"
#include "compiler.hpp"

#include "fmt/core.h"

namespace bytelox {

using enum InterpretResult;

VM::VM() { }

InterpretResult VM::interpret(std::string_view src) {
	chunk = new Chunk;
	Scanner scanner(src);
	Compiler compiler(scanner);
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

bool is_falsey(LoxValue value) {
	return value.is_nil() || (value.is_bool() && !value.as.boolean);
}

// inline?
LoxValue &VM::peek(size_t i) {
	return *(stack.rbegin() + i);
}

LoxValue &VM::peek() {
	return stack.back();
}

InterpretResult VM::run() {
	for (;;) {
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
			stack.push_back(chunk->constants[*ip++]);
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
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1).as.number += peek().as.number;
			stack.pop_back();
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
		case +OP::RETURN: {
			peek().print_value();
			stack.pop_back();
			fmt::print("\n");
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

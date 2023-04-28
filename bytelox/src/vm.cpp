#include "common.hpp"
#include "vm.hpp"
#include "debug.hpp"

#include "fmt/core.h"

namespace bytelox {

using enum InterpretResult;

VM::VM() {}

InterpretResult VM::interpret(Chunk *chunk) {
	this->chunk = chunk;
	ip = chunk->code.data();
	return run();
}

InterpretResult VM::run() {
	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		fmt::print("          ");
		for (LoxValue value : stack) {
			fmt::print("[ ");
			print_value(value);
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
		case +OP::ADD: {
			*(stack.rbegin()+1) += stack.back();
			stack.pop_back();
			break;
		}
		case +OP::SUB: {
			*(stack.rbegin()+1) -= stack.back();
			stack.pop_back();
			break;
		}
		case +OP::MUL: {
			*(stack.rbegin()+1) *= stack.back();
			stack.pop_back();
			break;
		}
		case +OP::DIV: {
			*(stack.rbegin()+1) /= stack.back();
			stack.pop_back();
			break;
		}
		case +OP::NEGATE: {
			stack.back() *= -1;
			break;
		}
		case +OP::RETURN: {
			print_value(stack.back());
			stack.pop_back();
			fmt::print("\n");
			return INTERPRET_OK;
		}
		}
	}
}

} // namespace bytelox

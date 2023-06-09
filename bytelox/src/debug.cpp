#include "debug.hpp"

#include "lox_value.hpp"
#include "lox_object.hpp"

#define FMT_HEADER_ONLY
#include "fmt/core.h"
#include <string>

namespace {
using namespace bytelox;

int simple_instruction(std::string_view name, int offset) {
	fmt::print("{}\n", name);
	return offset + 1;
}
int constant_instruction(std::string_view name, Chunk &chunk, int offset) {
	u8 index = chunk.code[offset + 1];
	fmt::print("{:<16} {:4} '", name, index);
	chunk.constants[index].print_value();
	fmt::print("'\n");
	return offset + 2;
}
int constant_long_instruction(std::string_view name, Chunk &chunk, int offset) {
	size_t index = chunk.code[offset+1] | (chunk.code[offset+2] << 8) | (chunk.code[offset+3] << 16);
	fmt::print("{:<16} {:4} '", name, index);
	chunk.constants[index].print_value();
	fmt::print("'\n");
	return offset + 4;
}

int invoke_instruction(std::string_view name, Chunk &chunk, int offset) {
	u8 constant = chunk.code[offset + 1];
	u8 arg_count = chunk.code[offset + 2];
	fmt::print("{:<16} ({} args) {:4} '", name, arg_count, constant);
	chunk.constants[constant].print_value();
	fmt::print("'\n");
	return offset + 3;
}

int byte_instruction(std::string_view name, Chunk &chunk, int offset) {
	u8 slot = chunk.code[offset + 1];
	fmt::print("{:<16} {:4}\n", name, slot);
	return offset + 2;
}

int jump_instruction(std::string_view name, int sign, Chunk &chunk, int offset) {
	u16 jump = *((u16 *) (&chunk.code[offset + 1])); // stored in little endian
	fmt::print("{:<16} {:4} -> {}\n", name, offset, offset + 1 + sign * jump);
	return offset + 3;
}

}

namespace bytelox {

int disassemble_instruction(Chunk &chunk, size_t offset) {
	fmt::print("{:0>4} ", offset);
	// bug, shouldn't be offset - 1
	if (offset > 0 && chunk.get_line(offset) == chunk.get_line(offset - 1)) {
		fmt::print("   | ");
	}
	else {
		fmt::print("{:>4} ", chunk.get_line(offset));
	}
	
	u8 instruction = chunk.code[offset];
	switch(instruction) {
		case +OP::CONSTANT:
			return constant_instruction("OP_CONSTANT", chunk, offset);
		case +OP::CONSTANT_LONG:
			return constant_long_instruction("OP_CONSTANT_LONG", chunk, offset);
		case +OP::NIL:
			return simple_instruction("OP_NIL", offset);
		case +OP::TRUE:
			return simple_instruction("OP_TRUE", offset);
		case +OP::FALSE:
			return simple_instruction("OP_FALSE", offset);
		case +OP::POP:
			return simple_instruction("OP_POP", offset);
		case +OP::GET_LOCAL:
			return byte_instruction("OP_GET_LOCAL", chunk, offset);
		case +OP::SET_LOCAL:
			return byte_instruction("OP_SET_LOCAL", chunk, offset);
		case +OP::GET_GLOBAL:
			return constant_instruction("OP_GET_GLOBAL", chunk, offset);
		case +OP::DEFINE_GLOBAL:
			return constant_instruction("OP_DEFINE_GLOBAL", chunk, offset);
		case +OP::SET_GLOBAL:
			return constant_instruction("OP_SET_GLOBAL", chunk, offset);
		case +OP::GET_UPVALUE:
			return byte_instruction("OP_GET_UPVALUE", chunk, offset);
		case +OP::SET_UPVALUE:
			return byte_instruction("OP_SET_UPVALUE", chunk, offset);
		case +OP::GET_PROPERTY:
			return constant_instruction("OP_GET_PROPERTY", chunk, offset);
		case +OP::SET_PROPERTY:
			return constant_instruction("OP_SET_PROPERTY", chunk, offset);
		case +OP::GET_SUPER:
			return constant_instruction("OP_GET_SUPER", chunk, offset);
		case +OP::EQUAL:
			return simple_instruction("OP_EQUAL", offset);
		case +OP::NOT_EQUAL:
			return simple_instruction("OP_NOT_EQUAL", offset);
		case +OP::GREATER:
			return simple_instruction("OP_GREATER", offset);
		case +OP::GREATER_EQUAL:
			return simple_instruction("OP_GREATER_EQUAL", offset);
		case +OP::LESS:
			return simple_instruction("OP_LESS", offset);
		case +OP::LESS_EQUAL:
			return simple_instruction("OP_LESS_EQUAL", offset);
		case +OP::ADD:
			return simple_instruction("OP_ADD", offset);
		case +OP::SUB:
			return simple_instruction("OP_SUB", offset);
		case +OP::MUL:
			return simple_instruction("OP_MUL", offset);
		case +OP::DIV:
			return simple_instruction("OP_DIV", offset);
		case +OP::NOT:
			return simple_instruction("OP_NOT", offset);
		case +OP::NEGATE:
			return simple_instruction("OP_NEGATE", offset);
		case +OP::PRINT:
			return simple_instruction("OP_PRINT", offset);
		case +OP::JUMP:
			return jump_instruction("OP_JUMP", 1, chunk, offset);
		case +OP::JUMP_IF_FALSE:
			return jump_instruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
		case +OP::LOOP:
			return jump_instruction("OP_LOOP", -1, chunk, offset);
		case +OP::CALL:
			return byte_instruction("OP_CALL", chunk, offset);
		case +OP::INVOKE:
			return invoke_instruction("OP_INVOKE", chunk, offset);
		case +OP::SUPER_INVOKE:
			return invoke_instruction("OP_SUPER_INVOKE", chunk, offset);
		case +OP::CLOSURE: {
			offset++;
			u8 constant = chunk.code.data()[offset++];
			fmt::print("{:<16} {:4} ", "OP_CLOSURE", constant);
			chunk.constants[constant].print_value();
			fmt::print("\n");
			ObjectFunction &fn = chunk.constants[constant].as.obj->as_function();
			for (int j=0; j<fn.upvalue_count; j++) {
				int is_local = chunk.code[offset++];
				int index = chunk.code[offset++];
				fmt::print("{:0>4}      |                     {} {}\n",
						offset - 2, is_local ? "local" : "upvalue", index);
			}
			return offset;
		}
		case +OP::CLOSE_UPVALUE:
			return simple_instruction("OP_CLOSE_UPVALUE", offset);
		case +OP::RETURN:
			return simple_instruction("OP_RETURN", offset);
		case +OP::CLASS:
			return constant_instruction("OP_CLASS", chunk, offset);
		case +OP::INHERIT:
			return simple_instruction("OP_INHERIT", offset);
		case +OP::METHOD:
			return constant_instruction("OP_METHOD", chunk, offset);
		default:
			fmt::print("Unknown opcode {}\n", instruction);
			return offset + 1;
	}
}

void disassemble_chunk(Chunk &chunk, std::string_view name) {
	fmt::print("== {} ==\n", name);
	for (size_t offset = 0; offset < chunk.count();) {
		offset = disassemble_instruction(chunk, offset);
	}
}

}

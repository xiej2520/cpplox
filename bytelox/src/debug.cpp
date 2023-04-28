#include "debug.hpp"

#include "lox_value.hpp"

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
	fmt::print("{:<16} {} '", name, index);
	print_value(chunk.constants[index]);
	fmt::print("'\n");
	return offset + 2;
}
int constant_long_instruction(std::string_view name, Chunk &chunk, int offset) {
	size_t index = chunk.code[offset+1] | (chunk.code[offset+2] << 8) | (chunk.code[offset+3] << 16);
	fmt::print("{:<16} {} '", name, index);
	print_value(chunk.constants[index]);
	fmt::print("'\n");
	return offset + 4;
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
		fmt::print("{:0>4} ", chunk.get_line(offset));
	}
	
	u8 instruction = chunk.code[offset];
	switch(instruction) {
		case +OP::CONSTANT:
			return constant_instruction("OP_CONSTANT", chunk, offset);
		case +OP::CONSTANT_LONG:
			return constant_long_instruction("OP_CONSTANT_LONG", chunk, offset);
		case +OP::ADD:
			return simple_instruction("OP_ADD", offset);
		case +OP::SUB:
			return simple_instruction("OP_SUB", offset);
		case +OP::MUL:
			return simple_instruction("OP_MUL", offset);
		case +OP::DIV:
			return simple_instruction("OP_DIV", offset);
		case +OP::NEGATE:
			return simple_instruction("OP_NEGATE", offset);
		case +OP::RETURN:
			return simple_instruction("OP_RETURN", offset);
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

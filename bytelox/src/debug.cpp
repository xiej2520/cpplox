#include "debug.hpp"

#include "lox_value.hpp"

#define FMT_HEADER_ONLY
#include "fmt/printf.h"
#include <string>

namespace {

int simple_instruction(std::string_view name, int offset) {
	fmt::printf("%s\n", name);
	return offset + 1;
}
int constant_instruction(std::string_view name, Chunk &chunk, int offset) {
	u8 constant = chunk.code[offset + 1];
	fmt::printf("%-16s %d '", name, constant);
	print_value(chunk.constants[constant]);
	fmt::print("'\n");
	return offset + 2;
}

}

int disassemble_instruction(Chunk &chunk, int offset) {
	fmt::printf("%04d ", offset);
	if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1]) {
		fmt::print("   | ");
	}
	else {
		fmt::printf("%4d ", chunk.lines[offset]);
	}
	
	u8 instruction = chunk.code[offset];
	switch(instruction) {
		case +OP::CONSTANT:
			return constant_instruction("OP_CONSTANT", chunk, offset);
		case +OP::RETURN:
			return simple_instruction("OP_RETURN", offset);
		default:
			fmt::printf("Unknown opcode %d\n", instruction);
			return offset + 1;
	}
}

void disassemble_chunk(Chunk &chunk, std::string_view name) {
	fmt::printf("== %s ==\n", name);
	for (size_t offset = 0; offset < chunk.count();) {
		offset = disassemble_instruction(chunk, offset);
	}
}

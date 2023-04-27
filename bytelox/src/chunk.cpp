#include "chunk.hpp"

void Chunk::write(u8 byte, int line) {
	code.push_back(byte);
	lines.push_back(line);
}

size_t Chunk::count() {
	return code.size();
}

size_t Chunk::add_constant(LoxValue value) {
	constants.push_back(value);
	return constants.size() - 1;
}

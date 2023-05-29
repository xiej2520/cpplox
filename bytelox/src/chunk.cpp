#include "chunk.hpp"

namespace bytelox {

void Chunk::write(u8 byte, u16 line) {
	code.push_back(byte);
	if (lines.empty() || lines.back().line != line) {
		lines.emplace_back(line, 1);
	}
	else {
		lines.back().count++;
	}
}

size_t Chunk::count() {
	return code.size();
}

u16 Chunk::get_line(int index) {
	for (RLE rle : lines) {
		index -= rle.count;
		if (index <= 0) {
			return rle.line;
		}
	}
	return lines.size();
}

size_t Chunk::add_constant(LoxValue value) {
	constants.push_back(value);
	return constants.size() - 1;
}

void Chunk::write_constant(LoxValue value, u16 line) {
	size_t index = constants.size();
	constants.push_back(value);
	if (index > UINT8_MAX) {
		write(+OP::CONSTANT_LONG, line);
		// little endian
		write(index & 0x0000FF, line);
		write(index & 0x00FF00, line);
		write(index & 0xFF0000, line);
	}
	else {
		write(+OP::CONSTANT, line);
		write(index, line);
	}
}

}

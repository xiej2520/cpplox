#pragma once

#include "common.hpp"
#include "lox_value.hpp"

#include <vector>

namespace bytelox {

// convert enum class to int with prepend +
template <typename T>
constexpr auto operator+(T e) noexcept
	-> std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>> {
	return static_cast<std::underlying_type_t<T>>(e);
}

enum class OP {
	CONSTANT,      // 2 bytes
	CONSTANT_LONG, // 4 bytes, 3 byte little-endian constant index
	ADD,           // 1 byte
	SUB,           // 1 byte
	MUL,           // 1 byte
	DIV,           // 1 byte
	NEGATE,        // 1 byte
	RETURN         // 1 byte
};

struct RLE {
	// at most 65535 lines
	// if line == 0, this means skip lines by count
	u16 line; 
	u16 count;
};

struct Chunk {
	std::vector<u8> code;
	std::vector<LoxValue> constants;
	std::vector<RLE> lines;
	void write(u8 byte, u16 line);

	size_t count();
	// gets the line of an instruction from its index
	u16 get_line(int index);
	size_t add_constant(LoxValue value);
	void write_constant(LoxValue value, u16 line);
};

}

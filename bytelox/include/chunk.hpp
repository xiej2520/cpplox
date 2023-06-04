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
	NIL,           // 1 byte
	TRUE,          // 1 byte
	FALSE,         // 1 byte
	POP,           // 1 byte
	GET_LOCAL,     // 1 byte
	SET_LOCAL,     // 1 byte
	GET_GLOBAL,    // 1 byte
	DEFINE_GLOBAL, // 1 byte
	SET_GLOBAL,    // 1 byte
	GET_UPVALUE,   // 
	SET_UPVALUE,   // 
	GET_PROPERTY,  //
	SET_PROPERTY,  //
	GET_SUPER,     //
	EQUAL,         // 1 byte
	NOT_EQUAL,     // 1 byte
	GREATER,       // 1 byte
	GREATER_EQUAL, // 1 byte
	LESS,          // 1 byte
	LESS_EQUAL,    // 1 byte
	ADD,           // 1 byte
	SUB,           // 1 byte
	MUL,           // 1 byte
	DIV,           // 1 byte
	NOT,           // 1 byte
	NEGATE,        // 1 byte
	PRINT,         // 1 byte
	JUMP,          // 3 bytes, 2 byte little endian offset
	JUMP_IF_FALSE, // 3 bytes, 2 byte little endian offset
	LOOP,          // 3 bytes, 2 byte little endian offset
	CALL,          // 1 byte
	INVOKE,        // 3 bytes, index of property name, num args
	SUPER_INVOKE,  // 3 bytes, index of property name, num args
	CLOSURE,       // Variable encoding: each upvalue encodes [is_local, index]
	CLOSE_UPVALUE, // 1 byte
	RETURN,        // 1 byte
	CLASS,         // 1 byte
	INHERIT,       //
	METHOD,        //
};

struct RLE {
	// at most 65535 lines
	// if line == 0, this means skip lines by count
	u16 line; 
	u16 count;
	RLE(u16 line, u16 count): line(line), count(count) {}
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

#pragma once

#include "chunk.hpp"

#include <string>
#include <vector>

namespace bytelox {

enum class InterpretResult {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
};

struct VM {
	Chunk *chunk;
	u8 *ip = nullptr; // next instruction to be executed
	std::vector<LoxValue> stack;

	VM();
	
	InterpretResult interpret(std::string_view src);
	
	InterpretResult run();
	

};

}

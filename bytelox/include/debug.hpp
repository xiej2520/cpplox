#pragma once

#include "chunk.hpp"

#include <string>

namespace bytelox {

int disassemble_instruction(Chunk &chunk, size_t offset);
void disassemble_chunk(Chunk &chunk, std::string_view name);

}

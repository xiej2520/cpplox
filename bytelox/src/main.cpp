#include "common.hpp"
#include "chunk.hpp"

#include "debug.hpp"

int main(int argc, const char *argv[]) {
	(void) argc;
	(void) argv;
	Chunk chunk;
	int constant = chunk.add_constant(1.2);
	chunk.write(+OP::CONSTANT, 123);
	chunk.write(constant, 123);
	chunk.write(+OP::RETURN, 123);
	disassemble_chunk(chunk, "test chunk");
	return 0;
}

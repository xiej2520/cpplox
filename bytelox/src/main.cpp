#include "common.hpp"
#include "chunk.hpp"
#include "vm.hpp"
#include "debug.hpp"

using namespace bytelox;

int main(int argc, const char *argv[]) {
	(void) argc;
	(void) argv;
	
	VM vm;

	Chunk chunk;
	int constant = chunk.add_constant(1.2);
	chunk.write(+OP::CONSTANT, 123);
	chunk.write(constant, 123);

	chunk.write_constant(3.4, 123);
	chunk.write(+OP::ADD, 123);
	chunk.write_constant(5.6, 123);
	chunk.write(+OP::DIV, 123);
	chunk.write(+OP::NEGATE, 123);
	chunk.write(+OP::RETURN, 123);
	
	
	disassemble_chunk(chunk, "test chunk");
	
	vm.interpret(&chunk);

	return 0;
}

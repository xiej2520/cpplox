#include "compiler.hpp"
#include "scanner.hpp"

#include "fmt/core.h"

namespace bytelox {

void compile(std::string_view src) {
	Scanner scanner(src);
	int line = -1;
	while (true) {
		Token token = scanner.scan_token();
		if (token.line != line) {
			fmt::print("{:4} ", token.line);
			line = token.line;
		}
		else {
			fmt::print("   | ");
		}
		fmt::print("{:2} '{:{}}'\n", +token.type, token.lexeme, token.lexeme.size());
		if (token.type == TokenType::END_OF_FILE) {
			break;
		}
	}
}

}

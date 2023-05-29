#include "common.hpp"
#include "chunk.hpp"
#include "vm.hpp"
#include "debug.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#define FMT_HEADER_ONLY
#include "fmt/core.h"

using namespace bytelox;

namespace {
	void run_repl(VM &vm) {
		while (true) {
			fmt::print("> ");
			std::string line;
			std::getline(std::cin, line);
			if (line.empty()) {
				break;
			}
			vm.interpret(line);
		}
	}
	
	std::string read_file(const std::string &path) {
		std::ifstream f(path);
		if (!f.is_open()) {
			fmt::print(stderr, "Could not open file \"{}\".\n", path);
			exit(74);
		}
		std::stringstream contents;
		contents << f.rdbuf();
		return contents.str();
	}
	
	void run_file(VM &vm, const std::string &path) {
		std::string src = read_file(path);
		InterpretResult res = vm.interpret(src);
		
		if (res == InterpretResult::INTERPRET_COMPILE_ERROR) {
			exit(65);
		}
		if (res == InterpretResult::INTERPRET_RUNTIME_ERROR) {
			exit(70);
		}
	}
}

int main(int argc, const char *argv[]) {
	VM vm;
	
	if (argc == 1) {
		run_repl(vm);
	}
	else if (argc == 2) {
		run_file(vm, argv[1]);
	}
	else {
		fmt::print(stderr, "Usage: lox [path]\n");
		exit(64);
	}
	return 0;
}

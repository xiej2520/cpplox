#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "token.h"
#include "scanner.h"
#include "parser.h"
#include "interpreter.h"
#include "resolver.h"

// circular dependency without
class RuntimeError;

namespace Lox {
	extern bool hadError;
	extern bool hadRuntimeError;

	void report(int line, const std::string &where, const std::string &msg);

	void error(int line, const std::string &msg);

	void error(Token token, const std::string &msg);
	
	void runtime_error(RuntimeError &err);

	void run(const std::string &src);

	void runFile(const std::string &path);

	void runPrompt();
}

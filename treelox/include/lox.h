#pragma once

#include <string>
#include <fstream>

class RuntimeError;
struct Token;

namespace Lox {
	extern bool had_error;
	extern bool had_runtime_error;

	void report(int line, std::string_view where, std::string_view msg);

	void error(int line, std::string_view msg);

	void error(const Token &token, std::string_view msg);
	
	void runtime_error(RuntimeError &err);

	void run(std::string_view src);

	void run_file(const std::string &path);

	void run_prompt();
}

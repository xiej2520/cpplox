#include "lox.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "interpreter.h"
#include "parser.h"
#include "resolver.h"
#include "scanner.h"
#include "token.h"

using enum TokenType;
using std::string_view;
using std::vector;

#include "astprinter.h"

namespace Lox {
	bool had_error = false;
	bool had_runtime_error = false;
	Interpreter interpreter;

	void report(int line, string_view where, string_view msg) {
		std::cerr << "[line " << line << "] Error" << where << ": " << msg << "\n";
		had_error = true;
	}

	void error(const Token &token, string_view msg) {
		if (token.type == END_OF_FILE) {
			report(token.line, " at end", msg);
		}
		else {
			report(token.line, " at '" + token.lexeme + "'", msg);
		}
	}

	void error(int line, string_view msg) {
		report(line, "", msg);
	}

	void runtime_error(RuntimeError &err) {
		std::cerr << err.what() << "\n[line " << err.token.line << "]" << "\n";
		had_runtime_error = true;
	}


	void run(string_view src) {
		Scanner scanner(src);
		vector<Token> tokens = scanner.scan_tokens();

		Parser parser(tokens);
		vector<Stmt> statements = parser.parse();
		if (had_error) {
			return;
		}

		Resolver resolver(interpreter);
		resolver.resolve(statements);

		if (had_error) {
			return;
		}
		interpreter.interpret(statements);
	}

	void repl_run(string_view src, Resolver &resolver) {
		Scanner scanner(src);
		vector<Token> tokens = scanner.scan_tokens();

		Parser parser(tokens);
		vector<Stmt> statements = parser.parse();
		if (had_error) {
			return;
		}

		resolver.resolve(statements);
		if (had_error) {
			return;
		}

		interpreter.repl_interpret(statements);
	}

	void run_file(const std::string &path) {
		std::ifstream f(path);
		std::stringstream contents;
		contents << f.rdbuf();
		run(contents.str());
		
		if (had_error) {
			std::exit(65); // BAD, FIX LATER
		}
		if (had_runtime_error) {
			std::exit(70);
		}
	}

	void run_prompt() {
		Resolver resolver(interpreter);
		while (true) {
			std::cout << "> ";
			std::string line;
			std::getline(std::cin, line);
			if (line.empty()) {
				break;
			}
			repl_run(line, resolver);
			std::cout << "\n";
			had_error = false;
		}
	}
}

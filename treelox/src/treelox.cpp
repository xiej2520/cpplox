#include "lox.h"

using enum TokenType;

namespace Lox {
	bool hadError = false;
	bool hadRuntimeError = false;
	Interpreter interpreter;

	void report(int line, const std::string &where, const std::string &msg) {
		std::cerr << "[line " << line << "] Error" << where << ": " << msg << "\n";
		hadError = true;
	}

	void error(Token token, const std::string &msg) {
		if (token.type == END_OF_FILE) {
			report(token.line, " at end", msg);
		}
		else {
			report(token.line, " at '" + token.lexeme + "'", msg);
		}
	}

	void error(int line, const std::string &msg) {
		report(line, "", msg);
	}

	void runtime_error(RuntimeError &err) {
		std::cerr << err.what() << "\n[line " << err.token.line << "]" << "\n";
		hadRuntimeError = true;
	}


	void run(const std::string &src) {
		Scanner scanner(src);
		std::vector<Token> tokens = scanner.scanTokens();

		Parser parser(tokens);
		std::vector<Stmt> statements = parser.parse();
		if (hadError) return;
		interpreter.interpret(statements);
	}

	void repl_run(const std::string &src) {
		Scanner scanner(src);
		std::vector<Token> tokens = scanner.scanTokens();

		Parser parser(tokens);
		std::vector<Stmt> statements = parser.parse();
		if (hadError) return;
		interpreter.repl_interpret(statements);
	}

	void runFile(const std::string& path) {
		std::ifstream f(path);
		std::stringstream contents;
		contents << f.rdbuf();
		run(contents.str());
		
		if (hadError) {
			std::exit(65); // BAD, FIX LATER
		}
		if (hadRuntimeError) {
			std::exit(70);
		}
	}

	void runPrompt() {
		while (true) {
			std::cout << "> ";
			std::string line;
			std::getline(std::cin, line);
			if (line.empty()) {
				break;
			}
			repl_run(line);
			std::cout << "\n";
			hadError = false;
		}
	}
}

#pragma once

#include "chunk.hpp"
#include "scanner.hpp"

#include <functional>
#include <string>

namespace bytelox {

enum class Precedence {
	NONE,
	ASSIGNMENT, // =
	OR,         // or
	AND,        // and
	EQUALITY,   // == !=
	COMPARISON, // < > <= >=
	TERM,       // + -
	FACTOR,     // * /
	UNARY,      // ! -
	CALL,       // . ()
	PRIMARY
};

struct ParseRule {
	std::function<void(void)> prefix;
	std::function<void(void)> infix;
	Precedence precedence;
};

constexpr int num_parse = 40;

struct Compiler {
	
	std::array<ParseRule, num_parse> rules;
	
	struct {
		Token previous;
		Token current;
		bool had_error = false;
		bool panic_mode = false;
	} parser;
	
	Scanner &scanner;
	
	Chunk *compiling_chunk = nullptr;
	Chunk *current_chunk();
	
	Compiler(Scanner &scanner);

	bool compile(std::string_view src, Chunk &chunk);
	void end_compiler();

	void advance();
	void consume(TokenType type, std::string_view msg);
	void emit_byte(u8 byte);
	void emit_bytes(u8 byte1, u8 byte2);
	void emit_constant(LoxValue value);
	void emit_return();
	
	u8 make_constant(LoxValue value);
	
	void expression();
	void number();
	void grouping();
	void unary();
	void binary();
	void literal();
	
	void parse_precedence(Precedence precedence);
	ParseRule *get_rule(TokenType type);
	
	void error_at_current(std::string_view msg);
	void error(std::string_view msg);
	void error_at(Token &token, std::string_view msg);

};

}

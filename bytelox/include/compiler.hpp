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
	std::function<void(bool)> prefix;
	std::function<void(bool)> infix;
	Precedence precedence;
};

constexpr int num_parse = 40;

struct VM;

struct Compiler {
	
	std::array<ParseRule, num_parse> rules;
	
	struct {
		Token previous;
		Token current;
		bool had_error = false;
		bool panic_mode = false;
	} parser;
	
	Scanner &scanner;
	VM &vm; // for adding LoxObject constants that need to have references for GC
	
	Chunk *compiling_chunk = nullptr;
	Chunk *current_chunk();
	
	Compiler(Scanner &scanner, VM &vm);

	bool compile(std::string_view src, Chunk &chunk);
	void end_compiler();

	void advance();
	void consume(TokenType type, std::string_view msg);
	bool match(TokenType type);
	bool check(TokenType type);
	void emit_byte(u8 byte);
	void emit_bytes(u8 byte1, u8 byte2);
	void emit_constant(LoxValue value);
	void emit_return();
	
	u8 make_constant(LoxValue value);
	
	void expression();
	void declaration();
	void statement();
	void var_declaration();
	
	void print_statement();
	void expression_statement();
	
	void number(bool);
	void grouping(bool);
	void unary(bool);
	void binary(bool);
	void literal(bool);
	void string(bool);
	void variable(bool can_assign);
	void named_variable(Token name, bool can_assign);
	
	void parse_precedence(Precedence precedence);
	u8 identifier_constant(const Token &name);
	u8 parse_variable(std::string_view msg);
	void define_variable(u8 global);
	ParseRule *get_rule(TokenType type);
	
	void error_at_current(std::string_view msg);
	void error(std::string_view msg);
	void error_at(Token &token, std::string_view msg);
	
	void synchronize();

};

}

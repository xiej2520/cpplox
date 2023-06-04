#pragma once

#include "chunk.hpp"
#include "scanner.hpp"
#include "lox_object.hpp"

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
	struct Local {
		Token name;
		int depth;
		bool is_captured;
	};
	
	enum class FunctionType {
		FUNCTION,
		INITIALIZER,
		METHOD,
		SCRIPT,
	};

	struct Upvalue {
		u8 index;
		bool is_local;
	};

	struct FunctionScope {
		FunctionScope *enclosing = nullptr;
		ObjectFunction *function = nullptr;
		FunctionType type = FunctionType::SCRIPT;
		Local locals[UINT8_MAX + 1];
		int local_count = 0;
		int scope_depth = 0;
		Upvalue upvalues[UINT8_MAX + 1];
		FunctionScope(Compiler &compiler, FunctionType type);
	};
	
	struct ClassScope {
		ClassScope *enclosing;
		bool has_superclass = false;
		ClassScope(ClassScope *enclosing): enclosing(enclosing) {}
	};
	
	std::array<ParseRule, num_parse> rules;
	
	struct {
		Token previous;
		Token current;
		bool had_error = false;
		bool panic_mode = false;
	} parser;
	
	Scanner &scanner;
	VM &vm; // for adding LoxObject constants that need to have references for GC
	FunctionScope *current_fn = nullptr;
	ClassScope *current_class = nullptr;
	
	Chunk *compiling_chunk = nullptr;
	Chunk *current_chunk();
	
	Compiler(Scanner &scanner, VM &vm);
	~Compiler();

	ObjectFunction *compile(std::string_view src);

	ObjectFunction *end_fn_scope();

	void advance();
	void consume(TokenType type, std::string_view msg);
	bool match(TokenType type);
	bool check(TokenType type);
	void emit_byte(u8 byte);
	void emit_bytes(u8 byte1, u8 byte2);
	void emit_loop(int loop_start);
	int emit_jump(u8 instruction);
	void emit_constant(LoxValue value);
	void patch_jump(int offset);
	void emit_return();
	
	u8 make_constant(LoxValue value);
	
	void begin_scope();
	void end_scope();
	
	void expression();
	void declaration();
	void statement();
	void var_declaration();
	void fun_declaration();
	void class_declaration();
	
	void print_statement();
	void expression_statement();
	void block();
	void if_statement();
	void while_statement();
	void for_statement();
	void function(FunctionType type);
	void return_statement();
	void method();
	
	void number(bool);
	void grouping(bool);
	void unary(bool);
	void binary(bool);
	void literal(bool);
	void string(bool);
	void variable(bool can_assign);
	void named_variable(Token name, bool can_assign);
	void and_(bool);
	void or_(bool);
	void call(bool);
	u8 argument_list();
	void dot(bool);
	void this_(bool);
	Token synthetic_token(std::string_view text);
	void super(bool);
	
	void parse_precedence(Precedence precedence);
	u8 identifier_constant(const Token &name);
	bool identifiers_equal(Token &a, Token &b);
	int resolve_local(FunctionScope &fs, Token &name);
	int add_upvalue(FunctionScope &fs, u8 index, bool is_local);
	int resolve_upvalue(FunctionScope &fs, Token &name);
	u8 parse_variable(std::string_view msg);
	void declare_variable();
	void define_variable(u8 global);
	void mark_initialized();
	void add_local(Token name);
	ParseRule *get_rule(TokenType type);
	
	void error_at_current(std::string_view msg);
	void error(std::string_view msg);
	void error_at(Token &token, std::string_view msg);
	
	void synchronize();
};

}

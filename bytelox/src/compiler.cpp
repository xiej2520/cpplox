#include "compiler.hpp"
#include "scanner.hpp"
#include "vm.hpp"

#ifdef DEBUG_PRINT_CODE
#include "debug.hpp"
#endif

#include "fmt/core.h"

namespace bytelox {

using ParseFn = void (*)();

Compiler::Compiler(Scanner &scanner, VM &vm): scanner(scanner), vm(vm) {
// everything needs to match signature with can_assign
#define RULE(method) [&](bool can_assign) { method(can_assign); }
	rules[+TokenType::LEFT_PAREN]    = {RULE(grouping), nullptr, Precedence::NONE};
	rules[+TokenType::RIGHT_PAREN]   = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::LEFT_BRACE]    = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::RIGHT_BRACE]   = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::COMMA]         = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::DOT]           = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::MINUS]         = {RULE(unary), RULE(binary), Precedence::TERM};
	rules[+TokenType::PLUS]          = {nullptr, RULE(binary), Precedence::TERM};
	rules[+TokenType::SEMICOLON]     = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::SLASH]         = {nullptr, RULE(binary), Precedence::FACTOR};
	rules[+TokenType::STAR]          = {nullptr, RULE(binary), Precedence::FACTOR};
	rules[+TokenType::BANG]          = {RULE(unary), nullptr, Precedence::NONE};
	rules[+TokenType::BANG_EQUAL]    = {nullptr, RULE(binary), Precedence::EQUALITY};
	rules[+TokenType::EQUAL]         = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::EQUAL_EQUAL]   = {nullptr, RULE(binary), Precedence::EQUALITY};
	rules[+TokenType::GREATER]       = {nullptr, RULE(binary), Precedence::COMPARISON};
	rules[+TokenType::GREATER_EQUAL] = {nullptr, RULE(binary), Precedence::COMPARISON};
	rules[+TokenType::LESS]          = {nullptr, RULE(binary), Precedence::COMPARISON};
	rules[+TokenType::LESS_EQUAL]    = {nullptr, RULE(binary), Precedence::COMPARISON};
	rules[+TokenType::IDENTIFIER]    = {RULE(variable), nullptr, Precedence::NONE};
	rules[+TokenType::STRING]        = {RULE(string), nullptr, Precedence::NONE};
	rules[+TokenType::NUMBER]        = {RULE(number), nullptr, Precedence::NONE};
	rules[+TokenType::AND]           = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::CLASS]         = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::ELSE]          = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::FALSE]         = {RULE(literal), nullptr, Precedence::NONE};
	rules[+TokenType::FOR]           = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::FUN]           = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::IF]            = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::NIL]           = {RULE(literal), nullptr, Precedence::NONE};
	rules[+TokenType::OR]            = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::PRINT]         = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::RETURN]        = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::SUPER]         = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::THIS]          = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::TRUE]          = {RULE(literal), nullptr, Precedence::NONE};
	rules[+TokenType::VAR]           = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::WHILE]         = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::ERROR]         = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::END_OF_FILE]   = {nullptr, nullptr, Precedence::NONE};
#undef RULE
}

Chunk *Compiler::current_chunk() {
	return compiling_chunk;
}

bool Compiler::compile(std::string_view src, Chunk &chunk) {
	Scanner scanner(src);
	compiling_chunk = &chunk;
	advance();
	
	while (!match(TokenType::END_OF_FILE)) {
		declaration();
	}
	return !parser.had_error;
}

void Compiler::end_compiler() {
	emit_return();
#ifdef DEBUG_PRINT_CODE
	if (!parser.had_error) {
		disassemble_chunk(*current_chunk(), "code");
	}
#endif
}

void Compiler::advance() {
	parser.previous = parser.current;
	while (true) {
		parser.current = scanner.scan_token();
		if (parser.current.type != TokenType::ERROR) {
			break;
		}
		error_at_current(parser.current.lexeme);
	}
}
void Compiler::consume(TokenType type, std::string_view msg) {
	if (parser.current.type == type) {
		advance();
		return;
	}
	error_at_current(msg);
}

bool Compiler::match(TokenType type) {
	if (!check(type)) return false;
	advance();
	return true;
}

bool Compiler::check(TokenType type) {
	return parser.current.type == type;
}

void Compiler::emit_byte(u8 byte) {
	current_chunk()->write(byte, parser.previous.line);
}
void Compiler::emit_bytes(u8 byte1, u8 byte2) {
	emit_byte(byte1);
	emit_byte(byte2);
}

void Compiler::emit_constant(LoxValue value) {
	emit_bytes(+OP::CONSTANT, make_constant(value));
}

void Compiler::emit_return() {
	emit_byte(+OP::RETURN);
}

u8 Compiler::make_constant(LoxValue value) {
	// replace with code for more than 255 constants
	size_t constant = current_chunk()->add_constant(value);
	if (constant > UINT8_MAX) {
		error("Too many constants in one chunk.");
		return 0;
	}
	return static_cast<u8>(constant);
}

void Compiler::expression() {
	parse_precedence(Precedence::ASSIGNMENT);
}

void Compiler::declaration() {
	if (match(TokenType::VAR)) {
		var_declaration();
	}
	else {
		statement();
	}
	
	if (parser.panic_mode) synchronize();
}

void Compiler::statement() {
	if (match(TokenType::PRINT)) {
		print_statement();
	}
	else {
		expression_statement();
	}
}

void Compiler::var_declaration() {
	u8 global = parse_variable("Expect Variable name.");
	if (match(TokenType::EQUAL)) {
		expression();
	}
	else {
		emit_byte(+OP::NIL);
	}
	consume(TokenType::SEMICOLON, "Expect ';' after variable declaration");
	define_variable(global);
}

void Compiler::print_statement() {
	expression();
	consume(TokenType::SEMICOLON, "Expect ';' after value.");
	emit_byte(+OP::PRINT);
}

void Compiler::expression_statement() {
	expression();
	consume(TokenType::SEMICOLON, "Expect ';' after expression.");
	emit_byte(+OP::POP);
}

void Compiler::number(bool) {
	double value = strtod(parser.previous.lexeme.begin(), nullptr);
	emit_constant(LoxValue(value));
}
void Compiler::grouping(bool) {
	expression();
	consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}
void Compiler::unary(bool) {
	TokenType operator_type = parser.previous.type;
	parse_precedence(Precedence::UNARY); // compile operand
	switch (operator_type) {
		case TokenType::BANG: emit_byte(+OP::NOT); break;
		case TokenType::MINUS: emit_byte(+OP::NEGATE); break;
		default: return;
	}
}
void Compiler::binary(bool) {
	TokenType operator_type = parser.previous.type;
	ParseRule *rule = get_rule(operator_type);
	parse_precedence(static_cast<Precedence>(+rule->precedence + 1));
	switch (operator_type) {
		case TokenType::BANG_EQUAL: emit_byte(+OP::NOT_EQUAL); break;
		case TokenType::EQUAL_EQUAL: emit_byte(+OP::EQUAL); break;
		case TokenType::GREATER: emit_byte(+OP::GREATER); break;
		case TokenType::GREATER_EQUAL: emit_byte(+OP::GREATER_EQUAL); break;
		case TokenType::LESS: emit_byte(+OP::LESS); break;
		case TokenType::LESS_EQUAL: emit_byte(+OP::LESS_EQUAL); break;
		case TokenType::PLUS:  emit_byte(+OP::ADD); break;
		case TokenType::MINUS: emit_byte(+OP::SUB); break;
		case TokenType::STAR:  emit_byte(+OP::MUL); break;
		case TokenType::SLASH: emit_byte(+OP::DIV); break;
		default: return; // unreachable
	}
}

void Compiler::literal(bool) {
	switch(parser.previous.type) {
		case TokenType::FALSE: emit_byte(+OP::FALSE); break;
		case TokenType::NIL: emit_byte(+OP::NIL); break;
		case TokenType::TRUE: emit_byte(+OP::TRUE); break;
		default: return; // unreachable
	}
}

void Compiler::string(bool) {
	emit_constant(vm.make_ObjectString(parser.previous.lexeme.substr(1, parser.previous.lexeme.size() - 2)));
}

void Compiler::variable(bool can_assign) {
	named_variable(parser.previous, can_assign);
}

void Compiler::named_variable(Token name, bool can_assign) {
	u8 arg = identifier_constant(name);
	if (can_assign && match(TokenType::EQUAL)) {
		expression();
		emit_bytes(+OP::SET_GLOBAL, arg);
	}
	else {
		emit_bytes(+OP::GET_GLOBAL, arg);
	}
}

void Compiler::parse_precedence(Precedence precedence) {
	advance();
	auto prefix_rule = get_rule(parser.previous.type)->prefix;
	if (prefix_rule == nullptr) {
		error("Expect expression.");
		return;
	}
	
	bool can_assign = precedence <= Precedence::ASSIGNMENT;
	prefix_rule(can_assign);

	while (precedence <= get_rule(parser.current.type)->precedence) {
		advance();
		auto infix_rule = get_rule(parser.previous.type)->infix;
		infix_rule(can_assign);
	}
	if (can_assign && match(TokenType::EQUAL)) {
		error("Invalid assignment target.");
	}
}

u8 Compiler::identifier_constant(const Token &name) {
	return make_constant(vm.make_ObjectString(name.lexeme));
}

u8 Compiler::parse_variable(std::string_view msg) {
	consume(TokenType::IDENTIFIER, msg);
	return identifier_constant(parser.previous);
}

void Compiler::define_variable(u8 global) {
	emit_bytes(+OP::DEFINE_GLOBAL, global);
}

ParseRule *Compiler::get_rule(TokenType type) {
	return &rules[+type];
}

void Compiler::error_at(Token &token, std::string_view msg) {
	if (parser.panic_mode) {
		return;
	}
	parser.panic_mode = true;
	fmt::print(stderr, "[line {}] Error", token.line);
	if (token.type == TokenType::END_OF_FILE) {
		fmt::print(stderr, " at end");
	}
	else if (token.type == TokenType::ERROR) {
		// Nothing
	}
	else {
		fmt::print(stderr, " at '{:{}}'", token.lexeme, token.lexeme.size());
	}
	fmt::print(stderr, ": {}\n", msg);
	parser.had_error = true;
}
void Compiler::error(std::string_view msg) {
	error_at(parser.previous, msg);
}
void Compiler::error_at_current(std::string_view msg) {
	error_at(parser.current, msg);
}

void Compiler::synchronize() {
	parser.panic_mode = false;
	while (parser.current.type != TokenType::END_OF_FILE) {
		if (parser.previous.type == TokenType::SEMICOLON) return;
		switch (parser.current.type) {
			case TokenType::CLASS:
			case TokenType::FUN:
			case TokenType::VAR:
			case TokenType::FOR:
			case TokenType::IF:
			case TokenType::WHILE:
			case TokenType::PRINT:
			case TokenType::RETURN:
				return;
			default: break;// do nothing
		}
		advance();
	}
}

}

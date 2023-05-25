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
	rules[+TokenType::AND]           = {nullptr, RULE(and_), Precedence::NONE};
	rules[+TokenType::CLASS]         = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::ELSE]          = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::FALSE]         = {RULE(literal), nullptr, Precedence::NONE};
	rules[+TokenType::FOR]           = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::FUN]           = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::IF]            = {nullptr, nullptr, Precedence::NONE};
	rules[+TokenType::NIL]           = {RULE(literal), nullptr, Precedence::NONE};
	rules[+TokenType::OR]            = {nullptr, RULE(or_), Precedence::NONE};
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
	current = new LocalState;
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
	current_chunk()->write(byte1, parser.previous.line);
	current_chunk()->write(byte2, parser.previous.line);
}

void Compiler::emit_loop(int loop_start) {
	emit_byte(+OP::LOOP);
	
	int offset = current_chunk()->code.size() - loop_start;
	if (offset > UINT16_MAX) {
		error("Loop body is too large.");
	}
	
	emit_bytes(offset & 0xFF, (offset >> 8) & 0xFF); // little endian
}

int Compiler::emit_jump(u8 instruction) {
	emit_byte(instruction);
	emit_bytes(0xFF, 0xFF);
	return current_chunk()->code.size()-2;
}

void Compiler::emit_constant(LoxValue value) {
	emit_bytes(+OP::CONSTANT, make_constant(value));
}

void Compiler::patch_jump(int offset) {
	int jump = current_chunk()->code.size() - offset; // no bytecode jump offset, taken care of
	if (jump > UINT16_MAX) {
		error("Too much code to jump over.");
	}
	
	// little endian
	current_chunk()->code[offset] = jump & 0xFF;
	current_chunk()->code[offset + 1] = (jump >> 8)  & 0xFF;
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

void Compiler::begin_scope() {
	current->scope_depth++;
}

void Compiler::end_scope() {
	current->scope_depth--;
	while (current->local_count > 0 &&
			current->locals[current->local_count-1].depth > current->scope_depth){
		// could be optimized to POPN
		emit_byte(+OP::POP);
		current->local_count--;
	}
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
	else if (match(TokenType::IF)) {
		if_statement();
	}
	else if (match(TokenType::WHILE)) {
		while_statement();
	}
	else if (match(TokenType::FOR)) {
		for_statement();
	}
	else if (match(TokenType::LEFT_BRACE)) {
		begin_scope();
		block();
		end_scope();
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

void Compiler::if_statement() {
	consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
	expression();
	consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
	
	int then_jump = emit_jump(+OP::JUMP_IF_FALSE);
	emit_byte(+OP::POP);
	statement();

	int else_jump = emit_jump(+OP::JUMP);
	patch_jump(then_jump);
	emit_byte(+OP::POP);
	if (match(TokenType::ELSE)) {
		statement();
	}
	patch_jump(else_jump);
}

void Compiler::while_statement() {
	int loop_start = current_chunk()->code.size();
	consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
	expression();
	consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
	
	int exit_jump = emit_jump(+OP::JUMP_IF_FALSE);
	emit_byte(+OP::POP);
	statement();
	emit_loop(loop_start);

	patch_jump(exit_jump);
	emit_byte(+OP::POP);
}

void Compiler::expression_statement() {
	expression();
	consume(TokenType::SEMICOLON, "Expect ';' after expression.");
	emit_byte(+OP::POP);
}

void Compiler::block() {
	while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::END_OF_FILE)) {
		declaration();
	}
	consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::for_statement() {
	begin_scope();
	consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
	if (match(TokenType::SEMICOLON)) {
		// no initializer
	}
	else if (match(TokenType::VAR)) {
		var_declaration();
	}
	else {
		expression_statement();
	}

	int loop_start = current_chunk()->code.size();
	int exit_jump = -1;
	if (!match(TokenType::SEMICOLON)) {
		expression();
		consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
		exit_jump = emit_jump(+OP::JUMP_IF_FALSE);
		emit_byte(+OP::POP);
	}
	
	if (!match(TokenType::RIGHT_PAREN)) {
		int body_jump = emit_jump(+OP::JUMP);
		int increment_start = current_chunk()->code.size();
		expression();
		emit_byte(+OP::POP);
		consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");
		
		emit_loop(loop_start);
		loop_start = increment_start;
		patch_jump(body_jump);
	}
	
	statement();
	emit_loop(loop_start);

	if (exit_jump != -1) {
		patch_jump(exit_jump);
		emit_byte(+OP::POP); // pop condition
	}

	end_scope();
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
	u8 get_op, set_op;
	int arg = resolve_local(*current, name);
	if (arg != -1) {
		get_op = +OP::GET_LOCAL;
		set_op = +OP::SET_LOCAL;
	}
	else {
		arg = identifier_constant(name);
		get_op = +OP::GET_GLOBAL;
		set_op = +OP::SET_GLOBAL;
	}
	if (can_assign && match(TokenType::EQUAL)) {
		expression();
		emit_bytes(set_op, static_cast<u8>(arg));
	}
	else {
		emit_bytes(get_op, static_cast<u8>(arg));
	}
}

void Compiler::and_(bool) {
	int end_jump = emit_jump(+OP::JUMP_IF_FALSE);
	emit_byte(+OP::POP);
	parse_precedence(Precedence::AND);
	patch_jump(end_jump);
}

// implement this in a faster way with OP::JUMP_IF_TRUE
void Compiler::or_(bool) {
	int else_jump = emit_jump(+OP::JUMP_IF_FALSE);
	int end_jump = emit_jump(+OP::JUMP);
	
	patch_jump(else_jump);
	emit_byte(+OP::POP);
	parse_precedence(Precedence::OR);
	patch_jump(end_jump);
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

bool Compiler::identifiers_equal(Token &a, Token &b) {
	return a.lexeme == b.lexeme;
}

int Compiler::resolve_local(LocalState &ls, Token &name) {
	for (int i=ls.local_count-1; i>=0; i--) {
		if (identifiers_equal(name, ls.locals[i].name)) {
			if (ls.locals[i].depth == -1) {
				error("Can't read local variable in its own initializer.");
			}
			return i;
		}
	}
	return -1;
}

u8 Compiler::parse_variable(std::string_view msg) {
	consume(TokenType::IDENTIFIER, msg);
	
	declare_variable();
	if (current->scope_depth > 0) return 0;

	return identifier_constant(parser.previous);
}

void Compiler::declare_variable() {
	if (current->scope_depth == 0) {
		return;
	}
	for (int i=current->local_count - 1; i>=0; i--) {
		Local *local = &current->locals[i];
		if (local->depth != -1 && local->depth < current->scope_depth) {
			break;
		}
		if (identifiers_equal(parser.previous, local->name)) {
			error("Already a variable with this name in this scope.");
		}
	}
	add_local(parser.previous);
}

void Compiler::define_variable(u8 global) {
	if (current->scope_depth > 0) {
		mark_initialized();
		// local variables are on top of the stack when allocated
		return;
	}
	emit_bytes(+OP::DEFINE_GLOBAL, global);
}

void Compiler::mark_initialized() {
	current->locals[current->local_count-1].depth = current->scope_depth;
}

void Compiler::add_local(Token name) {
	Local *local = &current->locals[current->local_count++];
	local->name = name;
	local->depth = -1; // uninitialized
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

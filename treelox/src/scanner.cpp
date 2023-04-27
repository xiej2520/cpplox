#include "scanner.h"

#include "lox.h"

using enum TokenType;

void Scanner::add_token(TokenType type, TokenLiteral literal) {
	std::string_view text = src.substr(start, current - start);
	tokens.emplace_back(type, text, literal, line);
}

void Scanner::add_token(TokenType type) {
	add_token(type, {.b=false});
}

bool Scanner::is_at_end() {
	return static_cast<size_t>(current) >= src.size();
}

char Scanner::advance() {
	return src[current++];
}

bool Scanner::match(char expected) {
	if (is_at_end() || src[current] != expected) {
		return false;
	}
	current++;
	return true;
}

char Scanner::peek() {
	return is_at_end() ? '\0' : src[current];
}
char Scanner::peek_next() {
	return (static_cast<size_t>(current) + 1 >= src.size()) ? '\0' : src[current + 1];
}
void Scanner::read_string() {
	while (peek() != '"' && !is_at_end()) {
		if (peek() == '\n') {
			line++;
		}
		advance();
	}
	if (is_at_end()) {
		Lox::error(line, "Unterminated string.");
		return;
	}
	advance(); // closing ""
	add_token(STRING, {.b=false});
}

void Scanner::read_number() {
	while (std::isdigit(peek())) {
		advance();
	}
	if (peek() == '.' && isdigit(peek_next())) {
		advance();
		while (std::isdigit(peek())) {
			advance();
		}
	}
	add_token(NUMBER, { .d=std::stod(std::string(src.substr(start, current - start))) });
}
void Scanner::read_identifier() {
	while (std::isalnum(peek()) || peek() == '_') {
		advance();
	}
	std::string_view text = src.substr(start, current - start);
	TokenType type = keywords.contains(text) ? keywords[text] : IDENTIFIER;
	if (type == BREAK) {
		if (peek() != ';') {
			Lox::error(Token{BREAK, text, {.b=0},  line}, "Expect ';' after break.");
		}
		add_token(type); // avoid lexeme being 'break '
		advance();
		return;
	}
	add_token(type);
}

void Scanner::scan_token() {
	char c = advance();
	switch (c) {
		case '(': add_token(LEFT_PAREN); break;
		case ')': add_token(RIGHT_PAREN); break;
		case '{': add_token(LEFT_BRACE); break;
		case '}': add_token(RIGHT_BRACE); break;
		case ',': add_token(COMMA); break;
		case '.': add_token(DOT); break;
		case '-': add_token(MINUS); break;
		case '+': add_token(PLUS); break;
		case ';': add_token(SEMICOLON); break;
		case '*': add_token(STAR); break;
		case '!': add_token(match('=') ? BANG_EQUAL : BANG); break;
		case '=': add_token(match('=') ? EQUAL_EQUAL : EQUAL); break;
		case '<': add_token(match('=') ? LESS_EQUAL : LESS); break;
		case '>': add_token(match('=') ? GREATER_EQUAL : GREATER); break;
		case '/':
			if (match('/')) {
				while (peek() != '\n' && !is_at_end())  {
					advance();
				}
			}
			else if (match('*')) {
				int stack = 1;
				while (!is_at_end() && stack > 0) {
					if (match('/')) {
						if (match('*')) {
							stack++;
						}
					}
					else if (match('*')) {
						if (match('/')) {
							stack--;
						}
					}
					else {
						advance();
					}
				}
			}
			else {
				add_token(SLASH);
			}
			break;
		case ' ':
		case '\r':
		case '\t': break;
		case '\n': line++; break;
		case '"': read_string(); break;
		default:
			if (isdigit(c)) {
				read_number();
			}
			else if (isalpha(c)) {
				read_identifier();
			}
			else {
				Lox::error(line, "Unexpected character.");
			}
			break;
	}
}

Scanner::Scanner(std::string_view src): src(src) {
	keywords["and"]    = AND;
	keywords["break"]  = BREAK;
	keywords["class"]  = CLASS;
	keywords["else"]   = ELSE;
	keywords["false"]  = FALSE;
	keywords["for"]		 = FOR;
	keywords["fun"]		 = FUN;
	keywords["if"]     = IF;
	keywords["nil"]    = NIL;
	keywords["or"]     = OR;
	keywords["print"]  = PRINT;
	keywords["return"] = RETURN;
	keywords["super"]  = SUPER;
	keywords["this"]   = THIS;
	keywords["true"]   = TRUE;
	keywords["var"]    = VAR;
	keywords["while"]  = WHILE;
}

std::vector<Token> Scanner::scan_tokens() {
	while (!is_at_end()) {
		start = current;
		scan_token();
	}
	tokens.emplace_back(END_OF_FILE, "", TokenLiteral{ .b=false }, line);
	return tokens;
}

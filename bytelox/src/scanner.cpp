#include "scanner.hpp"

#include <cstring> // memcmp

namespace bytelox {

using enum TokenType;

Scanner::Scanner(std::string_view src): src(src) { }

Token Scanner::scan_token() {
	skip_whitespace();
	start = current;
	if (is_at_end()) {
		return make_token(END_OF_FILE);
	}
	char c = advance();
	if (isalpha(c) || c == '_') {
		return identifier();
	}
	if (isdigit(c)) {
		return number();
	}
	switch (c) {
	case '(': return make_token(LEFT_PAREN);
	case ')': return make_token(RIGHT_PAREN);
	case '{': return make_token(LEFT_BRACE);
	case '}': return make_token(RIGHT_BRACE);
	case ';': return make_token(SEMICOLON);
	case ',': return make_token(COMMA);
	case '-': return make_token(MINUS);
	case '+': return make_token(PLUS);
	case '/': return make_token(SLASH);
	case '*': return make_token(STAR);
	case '!': return make_token(match('=') ? BANG_EQUAL : BANG);
	case '=': return make_token(match('=') ? EQUAL_EQUAL : EQUAL);
	case '<': return make_token(match('=') ? LESS_EQUAL : LESS);
	case '>': return make_token(match('=') ? GREATER_EQUAL : GREATER);
	case '"': return string();
	}
	return error_token("Unexcepted character.");
}

Token Scanner::make_token(TokenType type) {
	return {type, src.substr(start, current - start), line};
}

Token Scanner::error_token(std::string_view msg) {
	// only ever called with string literals
	return {ERROR, msg, line};
}

char Scanner::advance() {
	return src[current++];
}

char Scanner::peek() const {
	return src[current];
}

char Scanner::peek_next() const {
	if (is_at_end()) {
		return '\0';
	}
	return src[current + 1];
}

bool Scanner::match(char expected) {
	if (is_at_end() or src[current] != expected) {
		return false;
	}
	current++;
	return true;
}

void Scanner::skip_whitespace() {
	while (true) {
		switch (peek()) {
			case '\n':
				line++;
				// FALL THROUGH
			case ' ':
			case '\r':
			case '\t':
				advance();
				break;
			case '/':
				if (peek_next() == '/') {
					while (peek() != '\n' && !is_at_end()) {
						advance();
					}
				}
				else {
					return;
				}
				break;
			default:
				return;
		}
	}
}

bool Scanner::is_at_end() const {
	return current >= static_cast<int>(src.size());
}

Token Scanner::string() {
	while (peek() != '"' && !is_at_end()) {
		if (peek() == '\n') {
			line++;
		}
		advance();
	}
	if (is_at_end()) {
		return error_token("Unterminated string.");
	}
	advance(); // closing quote
	return make_token(STRING);
}

Token Scanner::number() {
	while (isdigit(peek())) {
		advance();
	}
	if (peek() == '.' && isdigit(peek_next())) {
		advance(); // consume '.'
		while (isdigit(peek())) {
			advance();
		}
	}
	return make_token(NUMBER);
}

Token Scanner::identifier() {
	// allow digits after first letter
	while (isalnum(peek()) || peek() == '_') {
		advance();
	}
	return make_token(identifier_type());
}

TokenType Scanner::identifier_type() {
	switch (src[start]) {
		case 'a': return current-start == sizeof("and")-1    && src[start+1] == 'n' && src[start+2] == 'd' ?  AND    : IDENTIFIER;
		case 'c': return current-start == sizeof("class")-1  && std::memcmp(&src[start+1], "lass", 4) == 0 ?  CLASS  : IDENTIFIER;
		case 'e': return current-start == sizeof("else")-1   && std::memcmp(&src[start+1], "lse", 3) == 0 ?   ELSE   : IDENTIFIER;
		case 'i': return current-start == sizeof("if")-1     && src[start+1] == 'f' ?                         IF     : IDENTIFIER;
		case 'n': return current-start == sizeof("nil")-1    && src[start+1] == 'i' && src[start+2] == 'l' ?  NIL    : IDENTIFIER;
		case 'o': return current-start == sizeof("or")-1     && src[start+1] == 'r' ?                         OR     : IDENTIFIER;
		case 'p': return current-start == sizeof("print")-1  && std::memcmp(&src[start+1], "rint", 4) == 0 ?  PRINT  : IDENTIFIER;
		case 'r': return current-start == sizeof("return")-1 && std::memcmp(&src[start+1], "eturn", 5) == 0 ? RETURN : IDENTIFIER; // NOLINT
		case 's': return current-start == sizeof("super")-1  && std::memcmp(&src[start+1], "uper", 4) == 0 ?  SUPER  : IDENTIFIER;
		case 'v': return current-start == sizeof("var")-1    && src[start+1] == 'a' && src[start+2] == 'r' ?  VAR    : IDENTIFIER;
		case 'w': return current-start == sizeof("while")-1  && std::memcmp(&src[start+1], "hile", 4) == 0 ?  WHILE  : IDENTIFIER;
		
		case 'f': {
			switch (current - start) {
				case 3: switch(src[start+1]) {
					case 'o': return src[start+2] == 'r' ? FOR : IDENTIFIER;
					case 'u': return src[start+2] == 'n' ? FUN : IDENTIFIER;
					default: return IDENTIFIER;
				}
				case 5: return std::memcmp(&src[start+1], "alse", 4) == 0 ? FALSE : IDENTIFIER;
				default: return IDENTIFIER;
			}
		}
		case 't': {
			if (current - start == 4) {
				switch (src[start+1]) {
					case 'h': return std::memcmp(&src[start+2], "is", 2) == 0 ? THIS : IDENTIFIER;
					case 'r': return std::memcmp(&src[start+2], "ue", 2) == 0 ? TRUE : IDENTIFIER;
					default: return IDENTIFIER;
				}
			}
			return IDENTIFIER;
		}
		default: return IDENTIFIER;
	}
}

}

#include "scanner.h"

using enum TokenType;

bool Scanner::isAtEnd() {
	return current >= src.size();
}

char Scanner::advance() {
	return src[current++];
}
void Scanner::addToken(TokenType type, LoxObject literal) {
	std::string text = src.substr(start, current - start);
	tokens.emplace_back(type, text, literal, line);
}

void Scanner::addToken(TokenType type) {
	addToken(type, 0);
}

bool Scanner::match(char expected) {
	if (isAtEnd()) return false;
	if (src[current] != expected) return false;
	current++;
	return true;
}

char Scanner::peek() {
	if (isAtEnd()) return '\0';
	return src[current];
}
char Scanner::peekNext() {
	if (current + 1 >= src.size()) return '\0';
	return src[current + 1];
}
void Scanner::read_string() {
	while (peek() != '"' && !isAtEnd()) {
		if (peek() == '\n') line++;
		advance();
	}
	if (isAtEnd()) {
		Lox::error(line, "Unterminated string.");
		return;
	}
	advance(); // closing ""
	std::string value = src.substr(start + 1, current - start - 2);
	addToken(STRING, value);
}
void Scanner::read_number() {
	while (std::isdigit(peek())) advance();
	if (peek() == '.' && isdigit(peekNext())) {
		advance();
		while (std::isdigit(peek())) advance();
	}
	addToken(NUMBER,
		std::stod(src.substr(start, current - start)));
}
void Scanner::read_identifier() {
	while (std::isalnum(peek())) advance();
	std::string text = src.substr(start, current - start);
	TokenType type = keywords.contains(text) ? keywords[text] : IDENTIFIER;
	addToken(type);
}

void Scanner::scanToken() {
	char c = advance();
	switch (c) {
		case '(': addToken(LEFT_PAREN); break;
		case ')': addToken(RIGHT_PAREN); break;
		case '{': addToken(LEFT_BRACE); break;
		case '}': addToken(RIGHT_BRACE); break;
		case ',': addToken(COMMA); break;
		case '.': addToken(DOT); break;
		case '-': addToken(MINUS); break;
		case '+': addToken(PLUS); break;
		case ';': addToken(SEMICOLON); break;
		case '*': addToken(STAR); break;
		case '!': addToken(match('=') ? BANG_EQUAL : BANG); break;
		case '=': addToken(match('=') ? EQUAL_EQUAL : EQUAL); break;
		case '<': addToken(match('=') ? LESS_EQUAL : LESS); break;
		case '>': addToken(match('=') ? GREATER_EQUAL : GREATER); break;
		case '/': if (match('/')) {
				while (peek() != '\n' && !isAtEnd()) advance();
			} else {
				addToken(SLASH);
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

Scanner::Scanner(std::string src): src(src) {
	keywords["and"]    = AND;
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

std::vector<Token> Scanner::scanTokens() {
	while (!isAtEnd()) {
		start = current;
		scanToken();
	}
	tokens.push_back(Token(END_OF_FILE, "", std::monostate{}, line));
	return tokens;
}

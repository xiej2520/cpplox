#pragma once
#include <exception>

#include "token.h"

// Lox runtime error, identical to std::runtime_error besides token
class RuntimeError : std::exception {
public:
	const Token token;
	std::string msg; // error message
	explicit RuntimeError(Token token, const char *msg);
	explicit RuntimeError(Token token, std::string &msg);
	// virtual for subclassing
	virtual ~RuntimeError() noexcept {}
	virtual const char *what() const noexcept {
		return msg.c_str();
	}
};

class ReturnUnwind : std::exception {
public:
	const LoxObject value;
	explicit ReturnUnwind(LoxObject value): value(value) {}
};

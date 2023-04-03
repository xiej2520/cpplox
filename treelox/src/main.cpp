#include <iostream>

#include "astprinter.h"
#include "scanner.h"
#include "treelox.h"

void testASTPrint() {
	Expr a = Literal(3);
	std::cout << print(a) << std::endl;
	Expr b = Grouping(std::make_shared<Expr>(Literal(4342)));
	std::cout << print(b) << std::endl;
	Expr e = Binary(
		std::make_shared<Expr>(Literal(3)),
		Token(TokenType::MINUS, "-", std::monostate{} ,1),
		std::make_shared<Expr>(Grouping(std::make_shared<Expr>(Literal(4342))))
	);
	std::cout << print(e) << std::endl;
	Expr expression = Binary(
		std::make_shared<Expr>(Unary(Token(TokenType::MINUS, "-", std::monostate{}, 1), std::make_shared<Expr>(Literal(123)))),
		Token(TokenType::STAR, "*", std::monostate{}, 1),
		std::make_shared<Expr>(Grouping(std::make_shared<Expr>(Literal(45.67))))
	);
	std::cout << print(expression) << "\n";
}

int main(int argc, char *argv[]) {
	std::vector<std::string> args(argc);
	for (int i=0; i<argc; i++) {
		args[i] = std::string(argv[i]);
	}
	//testASTPrint();

	if (args.size() > 2) {
		std::cout << "Usage: treelox [script]\n";
		return 0;
	}
	else if (args.size() == 2) {
		TreeLox::runFile(argv[1]);
	}
	else {
		TreeLox::runPrompt();
	}

	return 0;
}
